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
