#pragma once

// under this approach, version 1.4.1 (for example) would be:
// 10401
// so we're giving each decimal place 100 to work with
#define ESTD_BUILD_SEMVER(major, minor, patch)  ((major * 10000) + (minor * 100) + patch)