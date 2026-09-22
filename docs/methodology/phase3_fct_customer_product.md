# Phase 3A — fct_customer_product

## Business reason

Several project questions are not naturally customer-only or product-only questions. They depend on the relationship between a particular customer and a particular product: repeat behavior, habit strength, demand predictability, similarity, and cross-sell.

The mart establishes that reusable relationship grain before downstream analysis.

## Grain

**One row = one customer × product relationship observed in PRIOR history.**

Individual order-product records are collapsed into relationship-level history.

## Temporal boundary

The dataset provides PRIOR history plus held-out TRAIN/TEST outcomes. The mart uses PRIOR only. This protects downstream temporal evaluation from using information that belongs to the held-out outcome period.

## Core fields

- `purchase_count`
- `reorder_count`
- `reorder_rate`
- `first_purchase_order_number`
- `last_purchase_order_number`
- `purchase_span_orders`
- `customer_prior_order_count`
- `order_penetration`
- `avg_add_to_cart_order`

## Full-history versus predictive use

The full-history mart is appropriate for descriptive relationship intelligence. It is **not** a safe predictive feature source because the values summarize the complete PRIOR history. Future-oriented analyses therefore require an `as_of` layer constructed at explicit cutoffs.

## Validation

The implementation passes:

1. strict customer × product uniqueness;
2. exact reconciliation of `SUM(purchase_count)` to source PRIOR order-product rows;
3. no missing customer denominators;
4. reorder-rate bounds [0, 1];
5. penetration bounds (0, 1];
6. non-negative sequence span.

## Verified result

- 13,307,953 unique customer × product relationships
- 206,209 customers represented
- 49,677 products observed in PRIOR
- 32,434,489 source PRIOR order-product records reconciled exactly

## Next step

Build time-safe customer-product snapshots and customer behavior features. Those objects will support internal temporal backtesting before the original TRAIN outcome is used as the final benchmark.
