-- Grain: one row per customer.
-- PRIOR history only. This is a historical behavioral profile, not a final
-- prediction-time feature table.

CREATE OR REPLACE TABLE `project.dataset.mart_customer_behavior` AS
WITH order_stats AS (
  SELECT
    user_id,
    COUNT(*) AS prior_order_count,
    AVG(order_items) AS avg_basket_size,
    STDDEV_POP(order_items) AS basket_size_stddev
  FROM (
    SELECT
      o.user_id,
      o.order_id,
      COUNT(oi.product_id) AS order_items
    FROM `project.dataset.fct_orders` o
    JOIN `project.dataset.fct_order_items` oi
      USING (order_id)
    WHERE o.eval_set = 'prior'
      AND oi.source_set = 'prior'
    GROUP BY 1, 2
  )
  GROUP BY 1
),
repeat_stats AS (
  SELECT
    o.user_id,
    AVG(oi.reordered) AS reorder_rate,
    COUNT(DISTINCT oi.product_id) AS unique_products,
    COUNT(DISTINCT p.department_id) AS unique_departments
  FROM `project.dataset.fct_orders` o
  JOIN `project.dataset.fct_order_items` oi
    USING (order_id)
  JOIN `project.dataset.dim_product` p
    USING (product_id)
  WHERE o.eval_set = 'prior'
    AND oi.source_set = 'prior'
  GROUP BY 1
),
interval_stats AS (
  SELECT
    user_id,
    AVG(days_since_prior_order) AS avg_days_since_prior_order,
    STDDEV_POP(days_since_prior_order) AS days_since_prior_order_stddev,
    COUNTIF(days_since_prior_order = 30) AS capped_30_day_intervals,
    COUNT(days_since_prior_order) AS observed_intervals
  FROM `project.dataset.fct_orders`
  WHERE eval_set = 'prior'
  GROUP BY 1
)
SELECT
  os.user_id,
  os.prior_order_count,
  os.avg_basket_size,
  os.basket_size_stddev,
  rs.reorder_rate,
  rs.unique_products,
  rs.unique_departments,
  isx.avg_days_since_prior_order,
  isx.days_since_prior_order_stddev,
  isx.capped_30_day_intervals,
  isx.observed_intervals
FROM order_stats os
LEFT JOIN repeat_stats rs USING (user_id)
LEFT JOIN interval_stats isx USING (user_id);
