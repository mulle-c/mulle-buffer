### 1.1.3

* add missing file

### 1.1.2

* fix test a bit

### 1.1.1

* Various small improvements

## 1.1.0

* added `add_buffer` range functions


### 1.0.5

* remove obsolete file

### 1.0.4

* fix mingw, update sde

### 1.0.3

* modernized mulle-sde

### 1.0.2

* Various small improvements

### 1.0.1

* fix travis.yml

# 1.0.0

* migrated to mulle-sde
* made headernames hyphenated
* no longer distributed as a homebrew package

## 0.6.1

* rename mulle_buffer_dump_hex to mulle_buffer_hexdump and related


### 0.5.9

* support new mulle-tests

### 0.5.7

* fixed scion wrapper command

### 0.5.5

* follow mulle-configuration 3.1 changes and move .travis.yml to trusty

### 0.5.3

* Modernize cmake

### 0.4.9

* make cmake "C" project

### 0.4.7

* modernized project

## 0.4.1-5

* renamed inflexable methods to inflexible, but also kept old functions. So
there is no breaking API change. Marked old functions as deprecated. Need
new mulle_c11 for this.

### 0.3.1

* better asserts
* allow _mulle_buffer_done to run on an initialized buffer (still got to be
zeroed though)

### 0.2.1-7

* improve documentation
* rename some functions for release and for orthogonality with future releases
* call flush on done
* made `mulle_buffer_get_allocator` a non null returning function

### 0.1.1-3

* documentation improved somewhat

# 0.1

* moved it out of mulle-container, because it didn't belong there
