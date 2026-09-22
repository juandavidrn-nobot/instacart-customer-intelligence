CREATE OR REPLACE TABLE `project.dataset.fct_order_items` AS
SELECT
  oi.order_id,
  o.user_id,
  o.order_number,
  oi.product_id,
  oi.add_to_cart_order,
  oi.reordered,
  oi.source_set
FROM `project.dataset.stg_order_products` oi
JOIN `project.dataset.fct_orders` o
  USING (order_id);
