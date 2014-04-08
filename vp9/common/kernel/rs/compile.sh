#/bin/!bash
ndk_build_path=`which ndk-build`
ndk_path=`dirname $ndk_build_path`
api="19"
platform="arm"
input="inter_rs"
input1="inter_rs_h"
input2="inter_rs_v"
target="armv7"
c_input="vp9_rs_packing"
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

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/llvm-rs-cc -o ./obj/ -d ./obj/ -MD -reflect-c++ -I ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/clang/3.3/include -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/scriptc ${input}.rs

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/llvm-rs-cc -o ./obj/ -d ./obj/ -MD -reflect-c++ -I ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/clang/3.3/include -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/scriptc ${input1}.rs

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/llvm-rs-cc -o ./obj/ -d ./obj/ -MD -reflect-c++ -I ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/clang/3.3/include -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/scriptc ${input2}.rs

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/bcc_compat -O3 -o ./obj/${input}.bc.o -fPIC -shared -rt-path ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/libclcore.bc -mtriple ${target}-none-linux-gnueabi ./obj/${input}.bc

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/bcc_compat -O3 -o ./obj/${input1}.bc.o -fPIC -shared -rt-path ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/libclcore.bc -mtriple ${target}-none-linux-gnueabi ./obj/${input1}.bc

${ndk_path}/toolchains/renderscript/prebuilt/${arch}/bin/bcc_compat -O3 -o ./obj/${input2}.bc.o -fPIC -shared -rt-path ${ndk_path}/toolchains/renderscript/prebuilt/${arch}/lib/libclcore.bc -mtriple ${target}-none-linux-gnueabi ./obj/${input2}.bc

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -shared -Wl,-soname,librs.${input}.so -nostdlib ./obj/${input}.bc.o ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs/libcompiler_rt.a -o ./obj/librs.${input}.so -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs -lRSSupport -lm -lc

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -shared -Wl,-soname,librs.${input1}.so -nostdlib ./obj/${input1}.bc.o ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs/libcompiler_rt.a -o ./obj/librs.${input1}.so -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs -lRSSupport -lm -lc

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -shared -Wl,-soname,librs.${input2}.so -nostdlib ./obj/${input2}.bc.o ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs/libcompiler_rt.a -o ./obj/librs.${input2}.so -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -L ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib/rs -lRSSupport -lm -lc

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${input}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -fno-rtti -c ./obj/ScriptC_${input}.cpp -o ./obj/${input}.o

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${input1}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -fno-rtti -c ./obj/ScriptC_${input1}.cpp -o ./obj/${input1}.o

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${input2}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -fno-rtti -c ./obj/ScriptC_${input2}.cpp -o ./obj/${input2}.o

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${c_input}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti     -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -c ${c_input}.cpp -o ./obj/${c_input}.o

cp -f ${ndk_path}/sources/cxx-stl/stlport/libs/armeabi-v7a/libstlport_shared.so ./obj/

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -Wl,-soname,${c_input}.so -shared --sysroot=${ndk_path}/platforms/android-${api}/arch-${platform} ./obj/${c_input}.o ./obj/${input}.o ./obj/${input1}.o ./obj/${input2}.o -lgcc ./obj/libstlport_shared.so -no-canonical-prefixes -march=armv7-a -Wl,--fix-cortex-a8 -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/../lib/rs -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now  -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -ldl -llog -ljnigraphics -lRScpp_static -lcutils -lc -lm -o ${c_input}.so

echo "Done"
