-- BigQuery-style staging model
CREATE OR REPLACE TABLE `project.dataset.stg_orders` AS
SELECT
  CAST(order_id AS INT64) AS order_id,
  CAST(user_id AS INT64) AS user_id,
  CAST(eval_set AS STRING) AS eval_set,
  CAST(order_number AS INT64) AS order_number,
  CAST(order_dow AS INT64) AS order_dow,
  CAST(order_hour_of_day AS INT64) AS order_hour_of_day,
  CAST(days_since_prior_order AS FLOAT64) AS days_since_prior_order
FROM `project.dataset.raw_orders`;
