-- Grain: one row per customer x product, using PRIOR history only.
-- PURPOSE: create the behavioral substrate for lifecycle, demand, similarity,
-- and recommendation analyses without leaking held-out TRAIN information.

CREATE OR REPLACE TABLE `project.dataset.fct_customer_product` AS
WITH prior AS (
  SELECT
    o.user_id,
    oi.product_id,
    o.order_id,
    o.order_number,
    oi.reordered,
    oi.add_to_cart_order
  FROM `project.dataset.fct_order_items` oi
  JOIN `project.dataset.fct_orders` o
    USING (order_id)
  WHERE oi.source_set = 'prior'
),
aggregated AS (
  SELECT
    user_id,
    product_id,
    COUNT(*) AS purchase_count,
    SUM(reordered) AS reorder_count,
    SAFE_DIVIDE(SUM(reordered), COUNT(*)) AS reorder_rate,
    MIN(order_number) AS first_purchase_order,
    MAX(order_number) AS last_purchase_order,
    COUNT(DISTINCT order_id) AS orders_with_product
  FROM prior
  GROUP BY 1, 2
),
customer_orders AS (
  SELECT
    user_id,
    COUNT(*) AS prior_order_count
  FROM `project.dataset.fct_orders`
  WHERE eval_set = 'prior'
  GROUP BY 1
)
SELECT
  a.*,
  co.prior_order_count,
  SAFE_DIVIDE(a.orders_with_product, co.prior_order_count) AS customer_order_penetration
FROM aggregated a
JOIN customer_orders co
  USING (user_id);
