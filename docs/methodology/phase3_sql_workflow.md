# Phase 3 SQL Workflow

The project is SQL-first at the analytical-definition level.

The reusable business logic lives in `sql/03_marts/`. The local materialization uses an optimized execution path only because the raw source contains 32.4M order-product records and repeated CSV ingestion into local SQLite is inefficient. The SQL remains the source-of-truth definition for grain, joins, filters and aggregations.

For hands-on work, start with `sql/04_exploration/phase3b_behavior_walkthrough.sql`. Run each query, inspect the result, and ask which business hypothesis the result could support or contradict.

Recommended workflow for each query:

1. Identify the grain.
2. State the business question in plain English.
3. Run the query without deciding the conclusion first.
4. Check whether the observed pattern could have another explanation.
5. Convert only validated patterns into features or experiments.
