for file in *_tesselation_*; do mv "$file" "${file/_tesselation_/*_tessellation_*}"; done
