
Build with
```
make libs
```

Test with
```
CONNECTOVERRIDE_EXE=bin/connectoverride HOSTOVERRIDE_EXE=bin/hostoverride ruby -rrake -e 'Rake.application.run' test
```