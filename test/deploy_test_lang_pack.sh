sudo cp clientSideTesting/test_localisation_files/*.hh /usr/local/share/opendias/webcontent/
sudo cp clientSideTesting/test_localisation_files/i18n/*.hh /usr/local/share/opendias/
sudo cp clientSideTesting/test_localisation_files/includes/*.hh /usr/local/share/opendias/webcontent/includes/
sudo cp clientSideTesting/test_localisation_files/includes/local/*.hh /usr/local/share/opendias/webcontent/includes/local/
sudo perl -pi -e 's/<\/select>/<option value="hh">#### ########<\/option><\/select>/g' `grep -L '<option value="hh">#### ########' /usr/local/share/opendias/webcontent/includes/header.txt.* `
