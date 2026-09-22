-- Grain: one row per PRIOR order.

CREATE OR REPLACE TABLE `project.dataset.mart_basket_behavior` AS
WITH basket AS (
  SELECT
    o.order_id,
    o.user_id,
    o.order_number,
    o.order_dow,
    o.order_hour_of_day,
    o.days_since_prior_order,
    COUNT(*) AS basket_size,
    COUNT(DISTINCT oi.product_id) AS unique_products,
    COUNT(DISTINCT p.aisle_id) AS unique_aisles,
    COUNT(DISTINCT p.department_id) AS unique_departments,
    SUM(CASE WHEN oi.reordered = 1 THEN 1 ELSE 0 END) AS reordered_items,
    SUM(CASE WHEN oi.reordered = 0 THEN 1 ELSE 0 END) AS new_items
  FROM `project.dataset.fct_orders` o
  JOIN `project.dataset.fct_order_items` oi
    USING (order_id)
  JOIN `project.dataset.dim_product` p
    USING (product_id)
  WHERE o.eval_set = 'prior'
    AND oi.source_set = 'prior'
  GROUP BY 1,2,3,4,5,6
)
SELECT
  *,
  SAFE_DIVIDE(reordered_items, basket_size) AS reorder_share,
  SAFE_DIVIDE(new_items, basket_size) AS new_item_share
FROM basket;
