set -x
echo "Creating group and adding user to it. Logout/Login to ensure group add took effect"
sudo groupadd reminder
sudo usermod -aG reminder $USER

echo "Creating directories"
sudo mkdir /usr/local/etc/reminder.d
sudo chown -R :reminder /usr/local/etc/reminder.d
sudo chmod 770 /usr/local/etc/reminder.d

sudo mkdir /var/log/reminder.d
sudo chown -R :reminder /var/log/reminder.d
sudo chmod 770 /var/log/reminder.d

