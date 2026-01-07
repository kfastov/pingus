mergeInto(LibraryManager.library, {
  pingus_force_idbfs__deps: ['$IDBFS'],
  pingus_force_idbfs: function() {
    return 0;
  }
});
