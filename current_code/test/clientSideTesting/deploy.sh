#

BASEDIR="/usr/local/share/opendias"

mkdir -p $BASEDIR/webcontent/includes/tests/
cp clientTesting.html $BASEDIR/webcontent/clientTesting.html
cp jquery.qunit.css $BASEDIR/webcontent/style/jquery.qunit.css
cp jquery.qunit.js $BASEDIR/webcontent/includes/jquery.qunit.js
cd tests
cp testrunning.js $BASEDIR/webcontent/includes/tests/testrunning.js
cp listPageTest.js $BASEDIR/webcontent/includes/tests/listPageTest.js

cd ../
echo Now start opendias and visit http://host[:port]/opendias/clientTesting.html

