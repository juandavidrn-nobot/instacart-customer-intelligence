-- Phase 3B SQL walkthrough
-- Run these queries against the gold SQLite database or translate them to BigQuery.

-- 1) Confirm the grain: one row per order.
SELECT
  COUNT(*) AS rows,
  COUNT(DISTINCT order_id) AS distinct_orders
FROM mart_basket_behavior;

-- 2) Reconcile basket units to the source fact volume.
SELECT
  SUM(basket_size) AS basket_item_rows
FROM mart_basket_behavior;

-- Expected: 32,434,489.

-- 3) Inspect how basket behavior changes as customer history accumulates.
-- This is descriptive only; it is not yet a causal claim.
SELECT
  order_number,
  COUNT(*) AS orders,
  AVG(basket_size) AS avg_basket_size,
  AVG(reorder_share) AS avg_reorder_share,
  AVG(new_to_customer_share) AS avg_new_to_customer_share,
  AVG(days_since_prior_order) AS avg_days_since_prior_order
FROM mart_basket_behavior
GROUP BY order_number
ORDER BY order_number;

-- 4) Compare early, middle and mature history without assuming a result.
WITH tagged AS (
  SELECT
    *,
    CASE
      WHEN order_number BETWEEN 1 AND 3 THEN 'early'
      WHEN order_number BETWEEN 4 AND 10 THEN 'developing'
      WHEN order_number BETWEEN 11 AND 20 THEN 'mature'
      ELSE 'deep_history'
    END AS history_stage
  FROM mart_basket_behavior
)
SELECT
  history_stage,
  COUNT(*) AS orders,
  AVG(basket_size) AS avg_basket_size,
  AVG(reorder_share) AS avg_reorder_share,
  AVG(unique_departments) AS avg_unique_departments,
  AVG(unique_aisles) AS avg_unique_aisles
FROM tagged
GROUP BY history_stage
ORDER BY CASE history_stage
  WHEN 'early' THEN 1
  WHEN 'developing' THEN 2
  WHEN 'mature' THEN 3
  ELSE 4
END;

-- 5) Look for heterogeneity instead of relying only on averages.
SELECT
  basket_size,
  COUNT(*) AS orders
FROM mart_basket_behavior
GROUP BY basket_size
ORDER BY basket_size;

-- 6) First analytical warning: averages can hide very different histories.
-- Use customer-level trajectory features later to test whether these
-- differences persist within the same customer rather than only across people.
