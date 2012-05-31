sudo cp test_localisation_files/*.hh /usr/local/share/opendias/webcontent/
sudo cp test_localisation_files/i18n/*.hh /usr/local/share/opendias/
sudo cp test_localisation_files/includes/*.hh /usr/local/share/opendias/webcontent/includes/
sudo cp test_localisation_files/includes/local/*.hh /usr/local/share/opendias/webcontent/includes/local/
echo -e "Now add \n\t<option value='hh'>#### ########</option> \nto the select of id='language' in at least one translation of \n\twebcontent/includes/header.txt"
