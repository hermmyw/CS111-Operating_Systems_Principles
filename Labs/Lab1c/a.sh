if [ "${PATH:0:16}" == "/usr/local/cs/bin" ]
then
  true
else
  PATH=/usr/local/cs/bin:$PATH
fi

if ps | grep "simpsh"
then
  echo "simpsh is running in background."
  echo "Testing cannot continue."
  echo "Kill it and then run the script."
  exit 1
fi

if ! ls -lh pg98.txt
then
  echo "Please download pg98.txt!"
  exit 2
fi

echo "DO NOT run multiple testing scripts at the same time."
echo "Please check if there is any error message below."
echo "==="

# Check tarball.
STUDENT_UID="$1"
SUBMISSION="lab1-$STUDENT_UID.tar.gz"
if [ -e "$SUBMISSION" ]
then
  true
else
  echo "No submission: lab1-$STUDENT_UID.tar.gz"
  exit 1
fi

# Untar into student's directory.
rm -rf $STUDENT_UID
mkdir $STUDENT_UID
tar -C $STUDENT_UID -zxvf $SUBMISSION
cd $STUDENT_UID

# Make.
if [ -e "simpsh" ]
then
  rm -rf simpsh
fi
make || exit

make check
if [ $? == 0 ]
then
  echo "===>make check passed"
else
  echo "===>make check failed"
fi

rm -rf $SUBMISSION
make dist
if [ -e "$SUBMISSION" ]
then
  echo "===>make dist passed"
else
  echo "===>make dist failed"
fi

# Create testing directory.
TEMPDIR="lab1creadergradingtempdir"
rm -rf $TEMPDIR
mkdir $TEMPDIR
if [ "$(ls -A $TEMPDIR 2> /dev/null)" == "" ]
then
  true
else
  echo "Fatal error! The testing directory is not empty."
  exit 1
fi
mv simpsh ./$TEMPDIR/
cd $TEMPDIR


# Create testing files.
cat > a0.txt <<'EOF'
Hello world! CS 111! Knowledge crowns those who seek her.
EOF
cat a0.txt > a1.txt
cat a0.txt > a2.txt
cat a0.txt > a3.txt
cat a0.txt > a4.txt
cat a0.txt > a5.txt
cat a0.txt > a6.txt
cat a0.txt > a7.txt
cat a0.txt > a8.txt

cat > b0.txt <<'EOF'
FEAR IS THE PATH TO THE DARK SIDE...FEAR LEADS TO ANGER...ANGER LEADS TO HATE...HATE LEADS TO SUFFERING.
DO. OR DO NOT. THERE IS NO TRY.
EOF
cp b0.txt b1.txt

echo "==="
echo "Please DO NOT run multiple testing scripts at the same time."
echo "Make sure there is no simpsh running by you."
echo "Infinite waiting of simpsh due to unclosed pipe is unacceptable."
echo "Starting grading:"
NUM_PASSED=0
NUM_FAILED=0





echo -e "\n\n======ALERT! MANUAL CHECKING BEGINS======"

# Manual testing for 1c.
LOOP_INDEX_I=0
while [ $LOOP_INDEX_I -lt 100 ]
do
    cat ../../pg98.txt >> pg98_100.txt
    LOOP_INDEX_I=`expr $LOOP_INDEX_I + 1`
done
if ! ls -lh pg98_100.txt
then
  echo "Fatal error!"
  exit 3
fi


# Test case 26 --profile almost no time.
echo ""
echo "--->test case 26:"
echo "Check if there is time info for --rdwr option."
echo "Time should be (almost) 0"
./simpsh --creat --profile --rdwr test26io.txt >c26out.txt 2>c26err.txt
if [ $? == 0 ] && [ ! -s c26err.txt ] && [ -s c26out.txt ] && [ -e test26io.txt ] \
  && [ ! -s test26io.txt ] && wc -l < c26out.txt | grep -q "1"
then
  echo "----------c26out.txt----------"
  cat c26out.txt
  echo "----------c26out.txt----------"
else
  echo "===>test case 26 failed"
fi

# Test case 27 --profile scope.
echo ""
echo "--->test case 27:"
echo "Should only see time info for --rdwr, but not for --pipe or --command."
echo "And, we don't have wait here."
./simpsh --pipe --command 0 1 1 sleep 1 --creat --profile --rdwr test27io.txt \
  >c27out.txt 2>c27err.txt
if [ $? == 0 ] && [ ! -s c27err.txt ] && [ -s c27out.txt ] && [ -e test27io.txt ] \
  && [ ! -s test27io.txt ] && wc -l < c27out.txt | grep -q "1"
then
  echo "----------c27out.txt----------"
  cat c27out.txt
  echo "----------c27out.txt----------"
else
  echo "===>test case 27 failed"
fi

# Test case 28 --profile sort a large file.
echo ""
echo "--->test case 28:"
echo "Here you would see two time info, one for --command, which should be (almost) 0."
echo "Another time info, which is for child process, should not be 0."
echo "You can output time info for each child process, or just output a sum for"
echo "all children you waited for."
./simpsh --rdonly pg98_100.txt --creat --wronly test28out.txt --creat \
  --wronly test28err.txt --profile --command 0 1 2 sort --wait >c28out.txt \
  2>c28err.txt
if [ $? == 0 ] && [ ! -s c28err.txt ] && [ -s c28out.txt ] && [ -e test28out.txt ] \
  && [ -s test28out.txt ] && [ -e test28err.txt ] && [ ! -s test28err.txt ] \
  && (wc -l < c28out.txt | grep -q "4" || wc -l < c28out.txt | grep -q "5") 
then
  echo "----------c28out.txt----------"
  cat c28out.txt
  echo "----------c28out.txt----------"
else
  echo "===>test case 28 failed"
fi