# Phase 3B — Basket Behavior Mart

## Purpose

Build a reusable order-grain analytical mart that preserves the temporal and structural properties of each historical basket.

## Grain

One row = one PRIOR order.

This is intentionally different from `fct_customer_product`:

- `fct_customer_product` answers: **What is the long-run relationship between this customer and this product?**
- `mart_basket_behavior` answers: **What did this customer's basket look like at this point in their sequence?**

The second question is required for lifecycle trajectories, behavioral change and basket-structure analysis.

## Core fields

- `basket_size`: number of order-product records in the basket.
- `unique_products`: number of distinct products in the basket.
- `unique_aisles`: number of aisles represented.
- `unique_departments`: number of departments represented.
- `reordered_items`: products marked as previously reordered by the customer.
- `new_to_customer_items`: products in the basket with `reordered = 0`.
- `reorder_share`: reordered items / basket size.
- `new_to_customer_share`: new-to-customer items / basket size.
- `avg_add_to_cart_order`: average basket position; retained as a descriptive basket-structure field, not a preference ranking.
- `days_since_prior_order`: recurrence interval supplied by the source dataset; null on the customer's first order and capped at 30 by the source.

## Why this layer comes before predictive snapshots

The first predictive instinct would be to materialize an enormous customer × product × cutoff table. That is possible, but it is not the most efficient default representation. The order-grain basket mart preserves the chronological evidence at much lower materialization cost. Time-safe customer-product features can then be generated from explicit cutoffs when a particular experiment requires them.

This avoids creating large feature tables that are not yet tied to a validated business question.

## Validation results

- 3,214,874 mart rows.
- 32,434,489 total basket-size units, exactly reconciling to the PRIOR order-product source.
- 206,209 first-order rows, matching the customer count.
- 0 reorder-share or new-to-customer-share values outside [0,1].
- Basket size: median 8, P90 20, P99 35, maximum 145.

## Business uses

This mart supports:

1. Customer lifecycle and behavioral-change analysis.
2. Basket stability and exploration-vs-repetition analysis.
3. Product/basket structural analysis.
4. Demand predictability features such as basket-level recurrence and stability.

## Leakage rule

Only `eval_set = 'prior'` is included. The original `train` outcome remains reserved as the final external holdout.
