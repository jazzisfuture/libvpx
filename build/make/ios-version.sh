if [ "$1" == "--enable-shared" ];
then
    # Shared library framework builds are only possible on iOS 8 and later.
    echo "8.0"
else
    echo "6.0"
fi
