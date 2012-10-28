#

BASEDIR="/tmp/opendias_test/share/opendias"

cp test_localisation_files/*.hh $BASEDIR/webcontent/
cp test_localisation_files/i18n/*.hh $BASEDIR/
cp test_localisation_files/includes/*.hh $BASEDIR/webcontent/includes/
cp test_localisation_files/includes/local/*.hh $BASEDIR/webcontent/includes/local/
perl -pi -e 's/<\/select>/<option value="hh">#### ########<\/option><\/select>/g' `grep -L '<option value="hh">#### ########' $BASEDIR/webcontent/includes/header.txt.* `

mkdir -p $BASEDIR/webcontent/includes/tests/
cp clientTesting.html $BASEDIR/webcontent/clientTesting.html
cp jquery.qunit.css $BASEDIR/webcontent/style/jquery.qunit.css
cp jquery.qunit.js $BASEDIR/webcontent/includes/jquery.qunit.js
cd tests
cp *.js $BASEDIR/webcontent/includes/tests/

cd ../
echo Now start opendias and visit http://host[:port]/opendias/clientTesting.html

