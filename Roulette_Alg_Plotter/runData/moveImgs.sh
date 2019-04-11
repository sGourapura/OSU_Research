
dest=Run6_3.30.19_MutMod10
gifName="MutMod10.gif"


python3 makeGif.py 50 1 10

mkdir "$dest"
find -maxdepth 1 -name FScore\* -exec mv -t $dest {} +

mv myGifTest.gif "$dest/$gifName"