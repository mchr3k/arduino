Read this first
======

This project uses git submodules to include the Manchester library from ([arduino-libs-manchester](https://github.com/mchr3k/arduino-libs-manchester)). 
The correct way to interact with this is as follows. 

Getting started
------

Use one of the following:

      git clone --recursive <git url> .
  
Or

      git clone <git url> .
      git submodule update --init
      
Picking up a change to the Submodule
------

A git submodule refers to an external repository at an explicit revision. There are two ways to update the content of a submodule. 

      git submodule update libraries/Manchester

This will discard any changes in libraries/Manchester and fetch the current fixed revision associated with the submodule. 

      cd libraries/Manchester
      git pull origin master
      cd ..
      git add Manchester
      git commit -m 'pickup latest changes from external repo'
  
This is the correct approach to fetch the latest changes from the external repository. The git pull within the submodule folder does the fetching from the external repository. The subsequent git commit in the parent folder publishes into the enclosing repository that the revision of the submodule has been changed. 

Making changes to a submodule in place
------

Before you make any changes you have to modify the submodule checkout to actually refer to the master branch rather than a specific checkout.

      cd libraries/Manchester
      git checkout master
      
Once this is complete you can make changes and check them in as normal, treating libraries/Manchester as its own git repository. Any time you change the revision of the files within libraries/Manchester you should also choose whether to checkin the corresponding update to libraries/Manchester within the enclosing respository which still refers to an explicit revision.
