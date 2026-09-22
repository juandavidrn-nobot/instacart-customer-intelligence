# Limitations and transfer assumptions

1. The analysis uses the public Instacart Market Basket Analysis dataset as a proxy. It is not company-owned production data.
2. The dataset does not include prices, realized revenue, contribution margin, inventory, supplier lead times, delivery performance, subscription status, or intervention history.
3. Absolute revenue targets shown in the executive layer are business-case targets, not observed outcomes from the dataset.
4. Historical differences are observational and should not be described as causal treatment effects.
5. The source cadence field is capped at 30 days, so long intervals may be censored.
6. Production implementation should replace proxy thresholds with company-specific economics, lifecycle definitions, channel constraints, and randomized experiments.
