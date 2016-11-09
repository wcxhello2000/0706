

sudo /usr/local/nginx/sbin/nginx

#启动tracker程序
sudo /usr/bin/fdfs_trackerd ./conf/tracker.conf
sudo /usr/bin/fdfs_storaged /etc/fdfs/storage.conf

redis-server
#redis-cli
