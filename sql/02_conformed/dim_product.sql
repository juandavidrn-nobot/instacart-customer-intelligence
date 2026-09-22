CREATE OR REPLACE TABLE `project.dataset.dim_product` AS
SELECT
  p.product_id,
  p.product_name,
  p.aisle_id,
  a.aisle,
  p.department_id,
  d.department
FROM `project.dataset.stg_products` p
LEFT JOIN `project.dataset.stg_aisles` a
  ON p.aisle_id = a.aisle_id
LEFT JOIN `project.dataset.stg_departments` d
  ON p.department_id = d.department_id;
