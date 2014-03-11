#/bin/!bash
ndk_build_path=`which ndk-build`
ndk_path=`dirname $ndk_build_path`
api="19"
platform="arm"
target="armv7"
lib_name="libvp9rsif"
cpp_files=""
cpp_o_files=""

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

for f in $cpp_files; do
    fbase=${f%.*}
    ${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -MMD -MP -MF ./obj/${fbase}.o.d -fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -mthumb -Os -g -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs/cpp -I ${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/rs -I${ndk_path}/sources/cxx-stl/stlport/stlport -I${ndk_path}/sources/cxx-stl//gabi++/include -DANDROID  -Wa,--noexecstack -Wformat -Werror=format-security  -frtti     -I${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include -I../../../.. -c ${fbase}.cpp -o ./obj/${fbase}.o
done

cp -f ${ndk_path}/sources/cxx-stl/stlport/libs/armeabi-v7a/libstlport_shared.so ./obj/

${ndk_path}/toolchains/${platform}-linux-androideabi-4.6/prebuilt/${arch}/bin/${platform}-linux-androideabi-g++ -Wl,-soname,${lib_name}.so -shared --sysroot=${ndk_path}/platforms/android-${api}/arch-${platform} ${cpp_o_files} -lgcc ./obj/libstlport_shared.so -no-canonical-prefixes -march=armv7-a -Wl,--fix-cortex-a8 -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/include/../lib/rs -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now  -L${ndk_path}/platforms/android-${api}/arch-${platform}/usr/lib -ldl -llog -ljnigraphics -lRScpp_static -lcutils -lc -lm -o ${lib_name}.so

echo "Done"
