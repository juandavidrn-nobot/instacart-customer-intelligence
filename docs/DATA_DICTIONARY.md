# Data Dictionary — Executive Layer

## Source entities

| Entity | Grain | Purpose |
|---|---|---|
| orders | one row per order | Customer order history and order sequence |
| order_products__prior | one row per product in a prior order | Basket composition and reorder behavior |
| order_products__train | one row per product in the held-out train order | Future outcome / validation ground truth |
| products | one row per product | Product and department attributes |
| aisles | one row per aisle | Product hierarchy |
| departments | one row per department | Portfolio breadth and expansion analysis |

## Executive result fields

| Field | Definition |
|---|---|
| PROTECT target customers | Persistent behavioral deterioration population: 6,763 |
| PROTECT final signal | +1.87 days vs customer baseline in orders 26–30; +1.91 days vs baseline in orders 31–35 |
| GROW exposed customers | Holdout customers exposed to the validated product relationship: 847 |
| GROW adopters | 24 observed adopters |
| GROW baseline adoption | 0.463% |
| GROW exposed adoption | 2.834% |
| GROW adoption gap | +2.37 percentage points |
| EXPAND target customers | Customers with 20+ orders and <=10 departments by order 20, validated population: 4,758 |
| EXPAND target expansion | 64.84% |
| EXPAND non-target expansion | 53.71% |
| EXPAND expansion gap | +11.13 percentage points |
| Priority categories | Frozen +8.63 pp; Pantry +7.66 pp; Bakery +6.79 pp; Deli +6.68 pp |
