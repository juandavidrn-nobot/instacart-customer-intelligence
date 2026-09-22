# fct_customer_product

**Grain:** one row per `customer_id × product_id` relationship observed in the PRIOR history.

**Temporal role:** descriptive full-history development mart built only from the historical PRIOR split. It must not include the held-out TRAIN/TEST outcomes used for evaluation.

| Field | Meaning | Why it matters |
|---|---|---|
| `customer_id` | Customer identifier | Relationship key |
| `product_id` | Product identifier | Relationship key |
| `purchase_count` | Number of prior orders containing the product | Core relationship strength / frequency |
| `reorder_count` | Number of observed line-items marked reordered | Repeat behavior |
| `reorder_rate` | `reorder_count / purchase_count` | Product-specific repeat tendency |
| `first_purchase_order_number` | First customer order sequence in which product appears | Entry point into the relationship |
| `last_purchase_order_number` | Most recent observed customer order sequence in which product appears | Recency in sequence space |
| `purchase_span_orders` | Last minus first order number | Relationship longevity in order sequence |
| `customer_prior_order_count` | Number of observed PRIOR orders for the customer | Exposure denominator |
| `order_penetration` | `purchase_count / customer_prior_order_count` | Habit/penetration relative to customer history |
| `avg_add_to_cart_order` | Mean basket position when product was added | Basket-structure descriptor; not a preference ranking |

## What this table enables

This table gives us the customer-product relationship as an analytical object. It supports habit strength, repeat-demand analysis, customer similarity features, and cross-sell candidate generation.

## What this table does not enable safely

It must not be used directly as a predictive feature table for the held-out TRAIN outcome because several fields use the full observed PRIOR history. Predictive analyses require **as-of snapshots** constructed using only information available before the prediction point.
