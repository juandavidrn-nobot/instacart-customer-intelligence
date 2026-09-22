-- Combine prior and train at a canonical order-item grain.
CREATE OR REPLACE TABLE `project.dataset.stg_order_products` AS
SELECT
  CAST(order_id AS INT64) AS order_id,
  CAST(product_id AS INT64) AS product_id,
  CAST(add_to_cart_order AS INT64) AS add_to_cart_order,
  CAST(reordered AS INT64) AS reordered,
  'prior' AS source_set
FROM `project.dataset.raw_order_products_prior`

UNION ALL

SELECT
  CAST(order_id AS INT64) AS order_id,
  CAST(product_id AS INT64) AS product_id,
  CAST(add_to_cart_order AS INT64) AS add_to_cart_order,
  CAST(reordered AS INT64) AS reordered,
  'train' AS source_set
FROM `project.dataset.raw_order_products_train`;
