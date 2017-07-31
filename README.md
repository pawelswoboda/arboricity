Arboricity
========
The arboricity of a graph is the minimal number of forests covering a graph. The arboricity together with an associated forest cover can be computed efficiently, see e.g. the articles "Algorithms for Graphic Polymatroids and Parametric s-Sets" by Gabow or "Forests, Frames and Games: Algorithms for Matroid Sums and Applications" by Gabow and Westermann.

## Implementation ##
We provide a C++ implementation utilizing the efficient Boykov-Kolmogorov max flow algorithm for computing the arboricity. 

## Installation
Type `git clone https://github.com/pawelswoboda/LP_MP.git` for downloading and `cmake` for building, resulting in library `libarboricity.a`.
