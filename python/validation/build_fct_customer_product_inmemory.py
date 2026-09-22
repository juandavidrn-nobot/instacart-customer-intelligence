from pathlib import Path
import pandas as pd

ROOT=Path('/mnt/data')
RAW=ROOT/'instacart_raw'
OUT=ROOT/'instacart_project'/'data'/'gold'
OUT.mkdir(parents=True, exist_ok=True)

orders=pd.read_csv(RAW/'orders.csv',usecols=['order_id','user_id','eval_set','order_number'],dtype={'order_id':'int32','user_id':'int32','eval_set':'category','order_number':'int16'})
prior_orders=orders[orders.eval_set.eq('prior')][['order_id','user_id','order_number']].rename(columns={'user_id':'customer_id'}).set_index('order_id')

print('Reading prior...')
prior=pd.read_csv(RAW/'order_products__prior.csv',dtype={'order_id':'int32','product_id':'int32','add_to_cart_order':'int16','reordered':'int8'})
print('rows',len(prior),'memory MB',prior.memory_usage(deep=True).sum()/1e6)
print('Mapping orders...')
prior['customer_id']=prior['order_id'].map(prior_orders['customer_id']).astype('int32')
prior['order_number']=prior['order_id'].map(prior_orders['order_number']).astype('int16')

print('Aggregating...')
agg=(prior.groupby(['customer_id','product_id'],sort=False,observed=True)
 .agg(purchase_count=('order_id','size'),reorder_count=('reordered','sum'),first_purchase_order_number=('order_number','min'),last_purchase_order_number=('order_number','max'),add_to_cart_order_sum=('add_to_cart_order','sum'),add_to_cart_order_n=('add_to_cart_order','count'))
 .reset_index())
print('pairs',len(agg),'memory MB',agg.memory_usage(deep=True).sum()/1e6)

customer_counts=(prior_orders.reset_index().groupby('customer_id',observed=True).size().rename('customer_prior_order_count').reset_index())
agg=agg.merge(customer_counts,on='customer_id',how='left',validate='many_to_one')
agg['reorder_rate']=(agg['reorder_count']/agg['purchase_count']).astype('float32')
agg['purchase_span_orders']=(agg['last_purchase_order_number']-agg['first_purchase_order_number']).astype('int16')
agg['order_penetration']=(agg['purchase_count']/agg['customer_prior_order_count']).astype('float32')
agg['avg_add_to_cart_order']=(agg['add_to_cart_order_sum']/agg['add_to_cart_order_n']).astype('float32')

# Keep analytical columns only; raw sum/count are retained because they make avg auditable.
cols=['customer_id','product_id','purchase_count','reorder_count','reorder_rate','first_purchase_order_number','last_purchase_order_number','purchase_span_orders','customer_prior_order_count','order_penetration','avg_add_to_cart_order']
agg=agg[cols]

# CSV is interoperable; gzip is used for storage efficiency.
out=OUT/'fct_customer_product.csv.gz'
agg.to_csv(out,index=False,compression='gzip')
print('wrote',out, 'size MB',out.stat().st_size/1e6)

# validation report
metrics={
'source_prior_rows':len(prior),
'source_prior_distinct_orders':int(prior.order_id.nunique()),
'source_prior_distinct_customers':int(prior.customer_id.nunique()),
'mart_rows':len(agg),
'mart_distinct_customers':int(agg.customer_id.nunique()),
'mart_distinct_products':int(agg.product_id.nunique()),
'purchase_count_reconciles':int(agg.purchase_count.sum()==len(prior)),
'unmatched_customer_denominators':int(agg.customer_prior_order_count.isna().sum()),
'invalid_reorder_rate_rows':int(((agg.reorder_rate<0)|(agg.reorder_rate>1)).sum()),
'invalid_penetration_rows':int(((agg.order_penetration<=0)|(agg.order_penetration>1)).sum()),
}
(OUT/'fct_customer_product_validation.csv').write_text('metric,value\n'+ '\n'.join(f'{k},{v}' for k,v in metrics.items())+'\n')
print(metrics)
