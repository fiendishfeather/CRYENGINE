#START-FILE-LIST
# File List auto generated by waf2cmake.py, do not modify manually.

start_sources()
# -- zstd.waf_files --
sources_platform(ALL)
add_sources("NoUberFile"
	SOURCE_GROUP "Root"
		"../../SDKs/zstd-1.3.7/lib/common/bitstream.h"
		"../../SDKs/zstd-1.3.7/lib/common/debug.c"
		"../../SDKs/zstd-1.3.7/lib/common/entropy_common.c"
		"../../SDKs/zstd-1.3.7/lib/common/error_private.c"
		"../../SDKs/zstd-1.3.7/lib/common/error_private.h"
		"../../SDKs/zstd-1.3.7/lib/common/fse.h"
		"../../SDKs/zstd-1.3.7/lib/common/fse_decompress.c"
		"../../SDKs/zstd-1.3.7/lib/common/huf.h"
		"../../SDKs/zstd-1.3.7/lib/common/mem.h"
		"../../SDKs/zstd-1.3.7/lib/common/pool.c"
		"../../SDKs/zstd-1.3.7/lib/common/pool.h"
		"../../SDKs/zstd-1.3.7/lib/common/threading.c"
		"../../SDKs/zstd-1.3.7/lib/common/threading.h"
		"../../SDKs/zstd-1.3.7/lib/common/xxhash.c"
		"../../SDKs/zstd-1.3.7/lib/common/xxhash.h"
		"../../SDKs/zstd-1.3.7/lib/common/zstd_common.c"
		"../../SDKs/zstd-1.3.7/lib/common/zstd_errors.h"
		"../../SDKs/zstd-1.3.7/lib/common/zstd_internal.h"
		"../../SDKs/zstd-1.3.7/lib/dictBuilder/cover.c"
		"../../SDKs/zstd-1.3.7/lib/dictBuilder/divsufsort.c"
		"../../SDKs/zstd-1.3.7/lib/dictBuilder/fastcover.c"
		"../../SDKs/zstd-1.3.7/lib/dictBuilder/zdict.c"
		"../../SDKs/zstd-1.3.7/lib/dictBuilder/zdict.h"
		"../../SDKs/zstd-1.3.7/lib/compress/fse_compress.c"
		"../../SDKs/zstd-1.3.7/lib/compress/hist.c"
		"../../SDKs/zstd-1.3.7/lib/compress/huf_compress.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_compress.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_double_fast.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_double_fast.h"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_fast.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_fast.h"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_lazy.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_lazy.h"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_ldm.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_ldm.h"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_opt.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstd_opt.h"
		"../../SDKs/zstd-1.3.7/lib/compress/zstdmt_compress.c"
		"../../SDKs/zstd-1.3.7/lib/compress/zstdmt_compress.h"
		"../../SDKs/zstd-1.3.7/lib/decompress/huf_decompress.c"
		"../../SDKs/zstd-1.3.7/lib/decompress/zstd_decompress.c"
		"../../SDKs/zstd-1.3.7/lib/deprecated/zbuff.h"
		"../../SDKs/zstd-1.3.7/lib/deprecated/zbuff_common.c"
		"../../SDKs/zstd-1.3.7/lib/deprecated/zbuff_compress.c"
		"../../SDKs/zstd-1.3.7/lib/deprecated/zbuff_decompress.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_legacy.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v01.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v01.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v02.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v02.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v03.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v03.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v04.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v04.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v05.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v05.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v06.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v06.h"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v07.c"
		"../../SDKs/zstd-1.3.7/lib/legacy/zstd_v07.h"
)

end_sources()
#END-FILE-LIST

CryEngineStaticModule(zstd SOLUTION_FOLDER "Libs")

if (WIN32 OR DURANGO)
target_compile_definitions(${THIS_PROJECT} PRIVATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
endif()

#if (ORBIS OR DARWIN OR LINUX OR ANDROID)
#	target_compile_definitions(${THIS_PROJECT} PRIVATE -DHAVE_UNISTD_H)
#	target_compile_options(${THIS_PROJECT} PRIVATE -w)
#endif()
target_link_libraries( ${THIS_PROJECT} PRIVATE zlib)
target_include_directories(${THIS_PROJECT} PUBLIC "${SDK_DIR}/zstd-1.3.7/lib")
target_include_directories(${THIS_PROJECT} PUBLIC "${SDK_DIR}/zstd-1.3.7/lib/common")
