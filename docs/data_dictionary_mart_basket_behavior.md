# Data Dictionary — mart_basket_behavior

| Column | Grain meaning | Type | Notes |
|---|---|---|---|
| order_id | Unique historical order | integer | Primary key |
| customer_id | Customer placing the order | integer | Foreign key to customer |
| order_number | Customer's sequence number | integer | 1 = first observed order |
| order_dow | Day-of-week encoding from source | integer | Source-provided behavioral time variable |
| order_hour_of_day | Hour encoding from source | integer | Source-provided behavioral time variable |
| days_since_prior_order | Recurrence interval | numeric | Null for first order; source caps at 30 |
| first_order_flag | First order indicator | integer | 1 when order_number = 1 |
| basket_size | Number of order-product rows | integer | Primary basket-volume measure |
| unique_products | Distinct products | integer | Validates basket structure |
| unique_aisles | Distinct aisles represented | integer | Breadth of assortment |
| unique_departments | Distinct departments represented | integer | Category breadth |
| reordered_items | Items marked reordered | integer | Sum of source `reordered` flag |
| new_to_customer_items | Items not previously reordered | integer | basket_size - reordered_items |
| reorder_share | Share of basket reordered | numeric | Reordered items / basket size |
| new_to_customer_share | Share of basket new to customer | numeric | New items / basket size |
| avg_add_to_cart_order | Mean basket position | numeric | Descriptive only; not a preference ranking |
| max_add_to_cart_order | Last observed basket position | integer | Structural basket check |
