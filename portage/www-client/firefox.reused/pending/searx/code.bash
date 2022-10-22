
  if test "$SEARX" == "1"; then
    #cp --dereference --force -- "${srcdir}/searx.xml" "browser/locales/searchplugins/searx.xml"
    #cp --dereference --force -- "${srcdir}/searx.xml" "browser/components/search/searchplugins/searx.xml"
    mkdir -p "browser/components/search/extensions/searx" #oknvm-badthinking://without -p so we get an error if eg. extensions doesn't exist which would signal that another dirchange happened upstream(just like the above two commented out lines for .xml) in which case we need to make sure to add searx in the proper place, rather than just put it in the old place where it will be ignored by the new compiled firefox!
    cp --dereference --force -- "${srcdir}/searx_manifest.json" "browser/components/search/extensions/searx/manifest.json"
    cp --dereference --force -- "${srcdir}/searx_favicon.ico" "browser/components/search/extensions/searx/favicon.ico"
  fi

