#/bin/!bash
ndk_build_path=`which ndk-build`
ndk_path=`dirname $ndk_build_path`
api="19"
platform="arm"
target="armv7"
lib_name="libvp9rsif"
cpp_files=""
cpp_o_files=""
rs_files=""
rs_o_files=""

for k in *.rs; do
    rs_files="$k $rs_files"
    rs_o_files="$rs_o_files obj/"${k%.*}".o"
done

for f in *.cpp; do
    cpp_files="$f $cpp_files"
    cpp_o_files="$cpp_o_files obj/"${f%.*}".o"
done

toolchains=${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/linux-x86
if [ -d $toolchains ]
then
  arch=linux-x86
else
  arch=linux-x86_64
fi
echo "Start"

mkdir -p obj
cp -f ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs/libRSSupport.so ./obj/

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/arm-linux-androideabi-strip --strip-unneeded ./obj/libRSSupport.so

clcore_path=${ndk_path}/platforms/android-${api}/arch-arm/usr/lib/rs/libclcore.bc
if [ -f $clcore_path ]
then
  clcore_path=${ndk_path}/platforms/android-${api}/arch-arm/usr/lib/rs/libclcore.bc
else
  clcore_path=${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/libclcore.bc
fi

for k in $rs_files; do
    kbase="${k%.*}"
    echo $kbase
    ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/llvm-rs-cc -o ./obj/ -d ./obj/ -MD -reflect-c++ -I ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/clang/3.3/include -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/scriptc ${kbase}.rs
    ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/bcc_compat -O3 -o ./obj/${kbase}.bc.o -fPIC -shared -rt-path $clcore_path -mtriple ${target}-none-linux-gnueabi ./obj/${kbase}.bc
    ${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -shared -Wl,-soname,librs.${kbase}.so -nostdlib ./obj/${kbase}.bc.o ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs/libcompiler_rt.a -o ./obj/librs.${kbase}.so -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs -lRSSupport -lm -lc
    ${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${kbase}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -fno-rtti -c ./obj/ScriptC_${kbase}.cpp -o ./obj/${kbase}.o
done

for f in $cpp_files; do
    fbase=${f%.*}
    ${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${fbase}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti     -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -I../../../.. -c ${fbase}.cpp -o ./obj/${fbase}.o
done

cp -f ${ndk_path}/sources/cxx-stl/stlport/libs/armeabi-v7a/libstlport_shared.so ./obj/

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -Wl,-soname,${lib_name}.so -shared --sysroot=${ndk_path}/platforms/android-${api}/arch-${platform} ${rs_o_files} ${cpp_o_files} -lgcc ./obj/libstlport_shared.so -no-canonical-prefixes -march=armv7-a -Wl,--fix-cortex-a8 -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/../lib/rs -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now  -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -ldl -llog -ljnigraphics -lRScpp_static -lc -lm -o ${lib_name}.so

echo "Done"
