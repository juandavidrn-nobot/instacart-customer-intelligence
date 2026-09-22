# Phase 3 — Analytical Architecture

## 1. Objective

Create a reproducible analytical data model that supports the four business workstreams while preventing grain errors and temporal leakage.

## 2. Source entities

| Source | Grain | Role |
|---|---|---|
| `orders` | 1 row per order | Customer/order history, timing, evaluation split |
| `order_products__prior` | 1 row per product within a historical order | Historical transactional fact |
| `order_products__train` | 1 row per product within a held-out observed order | Ground-truth outcome for final evaluation |
| `products` | 1 row per product | Product reference |
| `aisles` | 1 row per aisle | Product hierarchy |
| `departments` | 1 row per department | Product hierarchy |

## 3. Target analytical model

```text
                         dim_product
                        /     |     \\
                       /      |      \\
                      /       |       \\
               dim_aisle   dim_department
                      \\       /
                       \\     /
                    fct_order_items
                           |
                        fct_orders
                           |
                       dim_customer

Additional behavioral marts:

fct_customer_product
mart_customer_behavior
mart_basket_behavior
mart_product_demand
mart_customer_similarity
ml_next_order_features
```

## 4. Canonical fact tables

### `fct_orders`
**Grain:** one row per order.

Required fields:
- `order_id`
- `user_id`
- `order_number`
- `order_dow`
- `order_hour_of_day`
- `days_since_prior_order`
- `eval_set`

### `fct_order_items`
**Grain:** one row per order × product.

Required fields:
- `order_id`
- `product_id`
- `add_to_cart_order`
- `reordered`
- `source_set`

`source_set` distinguishes historical `prior` rows from observed `train` rows.

## 5. Behavioral marts

### `fct_customer_product`
**Grain:** one row per customer × product.

Candidate metrics:
- purchase_count
- reorder_count
- reorder_rate
- first_purchase_order
- last_purchase_order
- orders_with_product
- customer_order_penetration
- consecutive_purchase_streak
- average interval (only where estimable without violating the time policy)
- interval variability

### `mart_customer_behavior`
**Grain:** one row per customer.

Candidate metrics:
- observed_order_count
- average_basket_size
- reorder_rate
- product_diversity
- department_diversity
- basket_stability
- order-interval characteristics
- history_depth

### `mart_basket_behavior`
**Grain:** one row per order.

Candidate metrics:
- basket_size
- unique_products
- unique_aisles
- unique_departments
- reordered_product_count
- new_product_count
- reorder_share
- basket similarity to previous order

### `mart_product_demand`
**Grain:** one row per product (with optional relative-order-period extensions).

Candidate metrics:
- purchase_volume
- unique_customer_count
- repeat_customer_count
- reorder_rate
- customer_concentration
- recurrence/stability measures
- demand predictability score components

### `mart_customer_similarity`
**Grain:** top-N customer peers per customer, not a full 206K × 206K matrix.

Purpose:
- peer-based cross-sell;
- recommendation experiments;
- behavioral similarity analysis.

## 6. Predictive feature policy

For prediction of an order at sequence `N`, all features must be computed from information available through order `N-1` only.

Allowed:
- past orders;
- past product purchases;
- past reorder behavior;
- historical basket statistics;
- historical customer/product affinities.

Forbidden:
- target order contents;
- future orders;
- aggregates computed using the customer's full history when that history includes the target period.

## 7. Evaluation design

Do not repeatedly tune against the original `train` outcome.

Preferred evaluation hierarchy:

```text
Historical prior orders
        ↓
Rolling/backtest splits within prior
        ↓
Model / method selection
        ↓
Lock methodology
        ↓
Original train orders as final benchmark
```

## 8. Important limitations

- No absolute calendar date in the dataset.
- `days_since_prior_order` is capped at 30.
- No inventory, supplier lead time, fulfillment, shipping, margin, or cost fields.
- Public dataset is a sample and does not establish universal population behavior.

## 9. Design principle

The architecture must make it possible to trace:

`raw field → transformation → feature → finding → business decision`

No final insight should depend on an undocumented transformation.
