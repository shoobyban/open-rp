#!/bin/sh

cd $HOME/open-rp || exit 1
svn update || exit 1
time make release || exit 1
svnrv=$(svn info | grep Revision: | cut -d' ' -f2)
archives=$(ls ORP*.zip | grep -v ORP-SVN)
for archive in $archives; do
	os=$(echo $archive | sed -e 's/^.*-SVN-\(.*\)\.zip$/\1/')
	snap=ORP-SVN-$svnrv-$os.zip
	[ -f $snap ] && break
	mv -v $archive $snap || exit 1
	scp -i $HOME/.ssh/id_rsa $snap orp@orp.ps3-hacks.com:
	ssh -i $HOME/.ssh/id_rsa orp@orp.ps3-hacks.com mv $snap /tmp
	rm -f ORP-SVN*.zip
	touch $snap
	break
done
echo "$0: done"
exit 0
