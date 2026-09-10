# Inherited components and corresponding source

NCAS's new code is GPL-3.0-or-later. The original notices in inherited sources remain authoritative for those components; the original project contains mixed notices. KhiCAS documentation describes GPL2, while individual Giac source files state GPL3-or-later. Do not remove notices or relabel every upstream component as solely NCAS code.

The project uses the full CG50 KhiCAS/Giac release from Bernard Parisse's Casio source directory, retrieved 2026-09-09. Primary authors include Bernard Parisse and Renée De Graeve; the original interface credits Gabrial Maia, Mike Smith and further calculator-project contributors. Consult the complete source headers for credits and licensing.

`source/giacbf.tgz` is the exact upstream source-and-object archive used by the default link. It includes Giac, the existing calculator GUI, big-number code, MicroPython library and upstream makefiles. Source archives for `libfxcg`, `ustl`, `tommath`, `micropy` and `python90` are included alongside it. These retain their upstream component licences, including applicable BSD-style, MIT, public-domain and GPL notices.

`support/` contains the exact custom support libraries and headers selected from the author's `casiolocal.tgz`, with the documented compatibility fixes. GCC runtime libraries retain their GCC Runtime Library Exception and GNU licensing terms. The dependency lock records the original archive hashes. This support directory is not an independently source-rebuilt SDK.

The Windows compiler and packaging tool are downloaded by `tools/prepare.cjs` from the official PrizmSDK 0.6 release. Their own sources, licences and notices are in the PrizmSDK/libfxcg upstream projects and compiler distributions. They are build tools, not new NCAS source.

Primary source directory: https://www-fourier.univ-grenoble-alpes.fr/~parisse/casio/

Upstream manual: https://www-fourier.univ-grenoble-alpes.fr/~parisse/casio/khicasioen.html

PrizmSDK: https://github.com/Jonimoose/libfxcg/releases/tag/v0.6
