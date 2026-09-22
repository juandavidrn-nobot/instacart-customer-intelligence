from __future__ import annotations

import os
import sqlite3
from pathlib import Path

import pandas as pd

ROOT = Path('/mnt/data')
RAW = ROOT / 'instacart_raw'
PROJECT = ROOT / 'instacart_project'
DB = PROJECT / 'data' / 'gold' / 'instacart_mart_build.sqlite'
DB.parent.mkdir(parents=True, exist_ok=True)

ORDERS_PATH = RAW / 'orders.csv'
PRIOR_PATH = RAW / 'order_products__prior.csv'

# Rebuild deterministically.
if DB.exists():
    DB.unlink()

orders = pd.read_csv(
    ORDERS_PATH,
    usecols=['order_id', 'user_id', 'eval_set', 'order_number'],
    dtype={
        'order_id': 'int32',
        'user_id': 'int32',
        'eval_set': 'string',
        'order_number': 'int16',
    },
)
prior_orders = orders.loc[orders['eval_set'].eq('prior'), ['order_id', 'user_id', 'order_number']].copy()
prior_orders = prior_orders.rename(columns={'user_id': 'customer_id'})
prior_order_map = prior_orders.set_index('order_id')

# Customer-level denominator: number of observed historical orders before held-out train/test.
customer_order_counts = (
    prior_orders.groupby('customer_id', as_index=False)
    .agg(customer_prior_order_count=('order_id', 'nunique'))
)

conn = sqlite3.connect(DB)
conn.execute('PRAGMA journal_mode=WAL;')
conn.execute('PRAGMA synchronous=NORMAL;')
conn.execute('PRAGMA temp_store=MEMORY;')

customer_order_counts.to_sql('dim_customer_prior_counts', conn, if_exists='replace', index=False)
conn.execute('CREATE UNIQUE INDEX idx_customer_counts ON dim_customer_prior_counts(customer_id)')

conn.execute('''
CREATE TABLE cp_partial (
    customer_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    purchase_count INTEGER NOT NULL,
    reorder_count INTEGER NOT NULL,
    first_order_number INTEGER NOT NULL,
    last_order_number INTEGER NOT NULL,
    add_to_cart_order_sum INTEGER NOT NULL,
    add_to_cart_order_n INTEGER NOT NULL
)
''')
conn.commit()

usecols = ['order_id', 'product_id', 'add_to_cart_order', 'reordered']
dtypes = {
    'order_id': 'int32',
    'product_id': 'int32',
    'add_to_cart_order': 'int16',
    'reordered': 'int8',
}

chunk_size = 2_000_000
processed = 0
chunk_id = 0

for chunk in pd.read_csv(PRIOR_PATH, usecols=usecols, dtype=dtypes, chunksize=chunk_size):
    chunk_id += 1
    # Join each order-product event to its customer and sequence number.
    # map() avoids carrying the entire orders table into every chunk merge.
    chunk['customer_id'] = chunk['order_id'].map(prior_order_map['customer_id'])
    chunk['order_number'] = chunk['order_id'].map(prior_order_map['order_number'])
    if chunk['customer_id'].isna().any() or chunk['order_number'].isna().any():
        bad = int(chunk['customer_id'].isna().sum() + chunk['order_number'].isna().sum())
        raise RuntimeError(f'Chunk {chunk_id}: {bad} unmatched order references')

    grouped = (
        chunk.groupby(['customer_id', 'product_id'], sort=False, observed=True)
        .agg(
            purchase_count=('order_id', 'size'),
            reorder_count=('reordered', 'sum'),
            first_order_number=('order_number', 'min'),
            last_order_number=('order_number', 'max'),
            add_to_cart_order_sum=('add_to_cart_order', 'sum'),
            add_to_cart_order_n=('add_to_cart_order', 'count'),
        )
        .reset_index()
    )
    grouped.to_sql('cp_partial', conn, if_exists='append', index=False)
    processed += len(chunk)
    print(f'chunk={chunk_id:02d} raw_rows={processed:,} partial_groups={len(grouped):,}', flush=True)

conn.execute('''
CREATE TABLE fct_customer_product AS
SELECT
    p.customer_id,
    p.product_id,
    SUM(p.purchase_count) AS purchase_count,
    SUM(p.reorder_count) AS reorder_count,
    CAST(SUM(p.reorder_count) AS REAL) / NULLIF(SUM(p.purchase_count), 0) AS reorder_rate,
    MIN(p.first_order_number) AS first_purchase_order_number,
    MAX(p.last_order_number) AS last_purchase_order_number,
    MAX(p.last_order_number) - MIN(p.first_order_number) AS purchase_span_orders,
    SUM(p.add_to_cart_order_sum) AS add_to_cart_order_sum,
    SUM(p.add_to_cart_order_n) AS add_to_cart_order_n
FROM cp_partial p
GROUP BY p.customer_id, p.product_id
''')

conn.execute('''
ALTER TABLE fct_customer_product ADD COLUMN customer_prior_order_count INTEGER;
''')
conn.execute('''
UPDATE fct_customer_product
SET customer_prior_order_count = (
    SELECT c.customer_prior_order_count
    FROM dim_customer_prior_counts c
    WHERE c.customer_id = fct_customer_product.customer_id
)
''')

conn.execute('''
ALTER TABLE fct_customer_product ADD COLUMN order_penetration REAL;
''')
conn.execute('''
UPDATE fct_customer_product
SET order_penetration = CAST(purchase_count AS REAL) / NULLIF(customer_prior_order_count, 0)
''')

conn.execute('''
ALTER TABLE fct_customer_product ADD COLUMN avg_add_to_cart_order REAL;
''')
conn.execute('''
UPDATE fct_customer_product
SET avg_add_to_cart_order = CAST(add_to_cart_order_sum AS REAL) / NULLIF(add_to_cart_order_n, 0)
''')

conn.execute('CREATE UNIQUE INDEX idx_fcp_customer_product ON fct_customer_product(customer_id, product_id)')
conn.execute('CREATE INDEX idx_fcp_product ON fct_customer_product(product_id)')
conn.execute('CREATE INDEX idx_fcp_customer ON fct_customer_product(customer_id)')
conn.commit()

# Validation metrics and reconciliation values.
metrics = {}
metrics['source_prior_rows'] = int(processed)
metrics['source_prior_distinct_orders'] = int(prior_orders['order_id'].nunique())
metrics['source_prior_distinct_customers'] = int(prior_orders['customer_id'].nunique())
metrics['mart_rows'] = int(conn.execute('SELECT COUNT(*) FROM fct_customer_product').fetchone()[0])
metrics['mart_distinct_customers'] = int(conn.execute('SELECT COUNT(DISTINCT customer_id) FROM fct_customer_product').fetchone()[0])
metrics['mart_distinct_products'] = int(conn.execute('SELECT COUNT(DISTINCT product_id) FROM fct_customer_product').fetchone()[0])
metrics['purchase_count_reconciles'] = int(conn.execute('''
SELECT COALESCE(SUM(purchase_count),0) = ? FROM fct_customer_product
''', (processed,)).fetchone()[0])
metrics['unmatched_customer_denominators'] = int(conn.execute('''
SELECT COUNT(*) FROM fct_customer_product WHERE customer_prior_order_count IS NULL
''').fetchone()[0])
metrics['invalid_reorder_rate_rows'] = int(conn.execute('''
SELECT COUNT(*) FROM fct_customer_product WHERE reorder_rate < 0 OR reorder_rate > 1
''').fetchone()[0])
metrics['invalid_penetration_rows'] = int(conn.execute('''
SELECT COUNT(*) FROM fct_customer_product WHERE order_penetration <= 0 OR order_penetration > 1
''').fetchone()[0])

(PROJECT / 'data' / 'gold' / 'fct_customer_product_validation.csv').write_text(
    'metric,value\n' + '\n'.join(f'{k},{v}' for k,v in metrics.items()) + '\n',
    encoding='utf-8',
)

conn.close()
print('VALIDATION')
for k, v in metrics.items():
    print(f'{k}: {v}')
print(f'DB: {DB}')
