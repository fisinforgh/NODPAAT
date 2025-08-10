# Create a temporary file to store the HTML content
for ((c = 2005; c <= 2025; c++)); do
  mkdir aura_$c
  cd aura_$c
  wget -q -O directory_listing.html https://acdisc.gsfc.nasa.gov/data/s4pa/Aura_OMI_Level3/OMTO3e.003/$c/
  grep -o 'href="[^"]*\.he5"' directory_listing.html |
    grep -v '\.1\.he5' |
    cut -d'"' -f2 |
    while read file; do
      echo "https://acdisc.gsfc.nasa.gov/data/s4pa/Aura_OMI_Level3/OMTO3e.003/$c/$file"
    done | sort | uniq >he5_files_$c.txt

  # Clean up the temporary file
  rm directory_listing.html

  # Show the number of files found
  echo "Found $(wc -l <he5_files_$c.txt) .he5 files. List saved to he5_files_$c.txt"

  # Preview the first few entries
  head -n 3 he5_files_$c.txt

  # Download the files with ariac
  aria2c -i he5_files_$c.txt -x 5 -j 5 -c
  cd ..
done
