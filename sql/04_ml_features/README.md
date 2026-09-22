# ML feature layer — design only at this stage

Features will be generated from an explicit prediction cutoff.

For a target order N, only information through N-1 may be used.

Candidate feature families:
- customer history depth;
- product purchase count;
- customer-product purchase count;
- customer-product reorder rate;
- product recency;
- department affinity;
- basket size and stability;
- cadence/interval features;
- peer/similarity features.

Final feature definitions will be frozen after backtesting design is implemented.
