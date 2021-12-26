:warning: WORK IN PROGRESS, NOTHING HAVE BEEN TESTED YET!

# SC16IS7XX
SC16IS7XX is a fully hardware independant **Driver** primarily aimed at embedded world

# Presentation
This driver only takes care of configuration and check of the internal registers and the formatting of the communication with the device. That means it does not directly take care of the physical communication, there is functions interfaces to do that.
Each driver's functions need a device structure that indicate with which device he must threat and communicate. Each device can have its own configuration.
This driver is compatible with:
* SC16IS740
* SC16IS741
* SC16IS741A
* SC16IS750
* SC16IS752
* SC16IS760
* SC16IS762

## Feature

This driver has been designed to:
* Be fully configurable (all features of the SC16IS7XX are managed)
* Manage Sleep mode
* Have no limit of configuration except the ones imposed by the device
* Manage devices completely independently
* Have an automatic baudrate setting
* Prevent all configuration errors

:warning: WORK IN PROGRESS, NOTHING HAVE BEEN TESTED YET!
