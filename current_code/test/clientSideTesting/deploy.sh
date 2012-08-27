#

BASEDIR="/usr/local/share/opendias"

cd ../
./deploy_test_lang_pack.sh

cd clientSideTesting
sudo mkdir -p $BASEDIR/webcontent/includes/tests/
sudo cp clientTesting.html $BASEDIR/webcontent/clientTesting.html
sudo cp jquery.qunit.css $BASEDIR/webcontent/style/jquery.qunit.css
sudo cp jquery.qunit.js $BASEDIR/webcontent/includes/jquery.qunit.js
cd tests
sudo cp *.js $BASEDIR/webcontent/includes/tests/

cd ../
echo Now start opendias and visit http://host[:port]/opendias/clientTesting.html

