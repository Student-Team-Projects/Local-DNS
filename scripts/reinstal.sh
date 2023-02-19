sudo mkdir temp
sudo cp /etc/local_dns/* ./temp
cd ..
sudo make local_dns
sudo make install
cd scripts
sudo cp ./temp/* /etc/local_dns
sudo rm -rf temp
