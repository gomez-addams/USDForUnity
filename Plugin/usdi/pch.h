﻿#pragma once

#ifdef _MSC_VER
    #define and     &&
    #define and_eq  &=
    #define bitand  &
    #define bitor   |
    #define compl   ~
    #define not     !
    #define not_eq  !=
    #define or      ||
    #define or_eq   |=
    #define xor     ^
    #define xor_eq  ^=
#endif
#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define BOOST_PYTHON_STATIC_LIB
    #define NOMINMAX

    #define BUILD_COMPONENT_SRC_PREFIX "pxr/"
    #define BUILD_OPTLEVEL_OPT
    #define TF_NO_GNU_EXT

    #define USD_ENABLE_CACHED_NEW
#endif

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <atomic>
#include <cstdarg>
#include <cmath>
#include <cctype>
#include <type_traits>
#ifdef usdiEnableBoostFilesystem
    #include <boost/filesystem.hpp>
#else
    #include <experimental/filesystem>
#endif
#include <tbb/tbb.h>

#pragma warning(push)
#pragma warning(disable:4100 4127 4244 4305)
#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/timeCode.h"
#include "pxr/usd/usd/treeIterator.h"
#include "pxr/usd/usd/variantSets.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/xformCommonAPI.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/points.h"
#include "pxr/base/gf/transform.h"
#include "pxr/base/gf/matrix2f.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/usd/ar/resolver.h"
#pragma warning(pop)

namespace usdi {
    class Context;
    class Attribute;
    class Schema;
    class Xform;
    class Camera;
    class Mesh;
    class Points;
} // namespace usdi

#pragma warning(disable:4201)
