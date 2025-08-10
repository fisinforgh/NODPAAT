# Create a temporary file to store the HTML content
for ((c = 1996; c <= 2005; c++)); do
  mkdir earth_$c
  cd earth_$c
  wget -q -O directory_listing.html https://acdisc.gsfc.nasa.gov/data/s4pa/EarthProbe_TOMS_Level3/TOMSEPL3dtoz.008/$c/

  grep -o 'href="[^"]*\.txt"' directory_listing.html |
    grep -v '\.1\.txt' |
    cut -d'"' -f2 |
    while read file; do
      echo "https://acdisc.gsfc.nasa.gov/data/s4pa/EarthProbe_TOMS_Level3/TOMSEPL3dtoz.008/$c/$file"
    done | sort | uniq >txt_files_$c.txt

  # Clean up the temporary file
  rm directory_listing.html

  # Show the number of files found
  echo "Found $(wc -l <txt_files_$c.txt) .txt files. List saved to txt_files_$c.txt"

  # Preview the first few entries
  head -n 3 txt_files_$c.txt

  # Download the files with ariac
  aria2c -i txt_files_$c.txt -x 5 -j 5 -c
  cd ..
done
