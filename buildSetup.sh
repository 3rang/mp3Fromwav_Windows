#!/bin/sh

PWD=$(pwd)

dpkg -s dialog

if [ $? -eq 0 ]; then
    echo "Package  is installed!"
else
    echo "Package  is NOT installed!"
	sudo apt-get install dialog
fi

dpkg -s whiptail

if [ $? -eq 0 ]; then
    echo "Package  is installed!"
else
    echo "Package  is NOT installed!"
	sudo apt-get install whiptail
fi
envir () {
  echo "/*********** setuping  environment...*************/"
  USER=`id -u`
  if [ "${USER}" -ne 0 ]; then
    SUDO=sudo
  fi

  echo "install dependencies:"
  echo "- make utility"
  echo "- cmake"
  echo "- pkg-config"
  echo "- minGW-w64"
  echo "- libmp3lame-dev"


    ${SUDO} apt-get -y install cmake \
      build-essential \
      pkg-config \
      mingw-w64 \
      libpthread-stubs0-dev \
      libglib2.0-dev \
      libmp3lame-dev

    echo "Enviroment is ready"

}

build () {

	if (whiptail --title "Build Setup" --yesno "Do you want to Compile setup " 10 60) then
  	echo "Compiling...."
		sleep 1
		cd ${PWD}/c/
  		make
  		cd -

	  cd ${PWD}/cpp/
	  make
	  cd -
	  echo "Build done"
	else
	  echo "You choose Not to build"
	fi

}

clean() {
	cd ${PWD}/c/bin/
	rm -rf *
	cd -
	 
	cd ${PWD}/cpp/bin/
	rm -rf *
	cd -
	
echo "clean done"
sleep 1
}
Run() {

	sleep 1	
	echo "To Run binary go to bin folder and run with .wav file path as argument (copy into window os)" 
	sleep 3
}

while :
do
	OPTION=$(whiptail --title "Build Setup " --menu "Choose your option for Building setup" 15 60 4 \
	"1" "Enviroment" \
	"2" "Build setup" \
	"3" "Clean setup" \
	"4" "Run " \
	"5" "EXit Setup"  3>&1 1>&2 2>&3 )

	exitstatus=$?

	if [ $exitstatus = 0 ]; then
    		echo "Your chosen option:" $OPTION
	else
		echo "You chose Cancel."
		exit 1
	fi
  case $OPTION in
    1)
      envir
      ;;
    2)
      build
      ;;
    3)
      clean
      ;;
    4)
      Run
      ;;
    5)
      exit 1
      ;;
    \?)
       exit 1
      ;;
  esac
done
