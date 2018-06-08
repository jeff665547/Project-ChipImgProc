\<Feature name>
===

# User story

## Motivation

- *Who need this feature? Developer or end-user?*
- *Why need this feature? A basic using case*
- *What similar product or library feature did you surveied*

## Workflow

*A basic caller(user or program) procedure, can be UI workflow, using case code or command script*

# Design plain

## Module description 

*This part can describe with UML, mind map, or directly give the interface code* 

## OS/platform dependent part

*Skip this part if the feature is following type*
- Embedded system.
- Hardware design.
- Any platform dependent project.

*Otherwise, think about following question*
- *Is this module need to access filesystem?*
- *Any kind of I/O?*
- *Is there any candidate cross platform solutions?*

## Dependency

* *Required third party libraries*
* *Use/used by any module in current design*
* *Any change required in current design. Open related issues.*

(Related implementation issue: #n)

# Test plain

## Unit test

### Items

1. *An item in spec*
2. *Another item in spec*

(Related test issue: #n)

### Mocks(if need)

* Mock feature 1.
* Mock feature 2.

(Related mock issue: #n)

## Integration test 

*If the feature is full independent to other parts of system, ignore this section*

### Items

1. *An item in spec*
2. *Another item in spec*

(Related test issue: #n)

### Resource

* *device, database etc.*


## System test

*Only required if this feature is a full runable application*

### All features

1. *feature name, issue number*
2. *feature name, issue number*

### Resource

* *device, database etc.*

(Related test issue: #n)

## Installation test

*Only required if this feature is a full runable application*

### Check list

* *Driver installed*
* *Environment variables*
* *Other system configuration*

### Resource

* *device, database etc.*

(Related test issue: #n)