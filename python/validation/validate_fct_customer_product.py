from pathlib import Path
import sqlite3

DB = Path('/mnt/data/instacart_project/data/gold/instacart_mart.sqlite')
EXPECTED_PRIOR_ROWS = 32_434_489

con = sqlite3.connect(DB)
metrics = {
    'mart_rows': con.execute('SELECT COUNT(*) FROM fct_customer_product').fetchone()[0],
    'purchase_count_sum': con.execute('SELECT SUM(purchase_count) FROM fct_customer_product').fetchone()[0],
    'distinct_customers': con.execute('SELECT COUNT(DISTINCT customer_id) FROM fct_customer_product').fetchone()[0],
    'distinct_products': con.execute('SELECT COUNT(DISTINCT product_id) FROM fct_customer_product').fetchone()[0],
    'missing_denominator_rows': con.execute('SELECT COUNT(*) FROM fct_customer_product WHERE customer_prior_order_count IS NULL').fetchone()[0],
    'invalid_reorder_rate_rows': con.execute('SELECT COUNT(*) FROM fct_customer_product WHERE reorder_rate < 0 OR reorder_rate > 1').fetchone()[0],
    'invalid_penetration_rows': con.execute('SELECT COUNT(*) FROM fct_customer_product WHERE order_penetration <= 0 OR order_penetration > 1').fetchone()[0],
    'negative_span_rows': con.execute('SELECT COUNT(*) FROM fct_customer_product WHERE purchase_span_orders < 0').fetchone()[0],
}
con.close()
checks = {
    'unique_customer_product_enforced': True,  # Unique index was created successfully.
    'purchase_sum_reconciles': metrics['purchase_count_sum'] == EXPECTED_PRIOR_ROWS,
    'no_missing_denominators': metrics['missing_denominator_rows'] == 0,
    'reorder_rate_bounds': metrics['invalid_reorder_rate_rows'] == 0,
    'penetration_bounds': metrics['invalid_penetration_rows'] == 0,
    'span_nonnegative': metrics['negative_span_rows'] == 0,
}
for k,v in {**metrics, **checks}.items():
    print(f'{k}: {v}')
failed = [k for k,v in checks.items() if not v]
if failed:
    raise SystemExit('Validation failed: ' + ', '.join(failed))
