# Ping Bluetooth LE

## Reqirements

The sample supports the following development kits:

|Hardware platforms|PCA|Board name|Build target|
|---|---|---|---|
|nrf52840 DK|PCA10056|nrf52840dk_nrf52840|nrf52840dk_nrf52840|
|nrf5340 DK|PCA10095|nrf5340dk_nrf5340|nrf5340dk_nrf5340_cpuapp|

---

## Overview

The sample sends the message to aws it recieves with use of Bluetooth LE. The sample also sends a message to aws when it connects for the first time.

---

## Building and running

## Sample process

Sample can be found in the directory where this file first was.

> [!NOTE]
>
> Before you flash you Sidewalk sample, make sure you completed the following:
> * You downloaded the Sidewalk repository and updated west according to the [Downloading the Sidewalk repository](https://nrfconnect.github.io/sdk-sidewalk/setting_up_sidewalk_environment/setting_up_sdk.html#dk-building-sample-app) section.
> * You provisioned your device during the [Setting up your Sidewalk product](https://nrfconnect.github.io/sdk-sidewalk/setting_up_sidewalk_environment/setting_up_sidewalk_product.html#setting-up-sidewalk-product).
> 
> This step needs to be completed only once. You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the [Building and programming an application](#) documentation. 

This application can be built as follows:

```
west build -b <supported_build_target> <path_to_the_sample>
```

If path to the sample is left empty it defaults to the folder you're currently in 

Example:

```
west build -b nrf52840dk_nrf52840
```

```mermaid
flowchart TD
    St(Start)
    St --> Ma[Main]
    subgraph Ma_t [main_thread]
    Ma
    end
    Ma --> Sid_s[sidewalk_start]
    subgraph Sid_t [sid_q_thread]
    Sid_s -->| queue_init#40;#41; | IL[IDLE]
    IL -.->|sidewalk_event| Sid_e{what sidewalk_event?}
    Sid_e -->|on_status_changed| Sid_pr
    Sid_e -->|on_message_recieved| Q_ph[queue_push#40;msg#41;]
    Q_ph --> Sid_pr[sidewalk_process_event]
    Sid_pr --> Mq_e1{are there messeges on queue?}
    Mq_e1 -->|No| IL
    Conn{is there a connection?} & Sid_conn[sidewalk_connection_request]
    Mq_e1 -->|Yes| Conn
    Conn -->|No| Sid_conn
    Sid_conn --> IL
    Conn -->|Yes| Sid_send[sidewalk_send_message]
    Sid_send --> Q_pp[queue_pop#40;#41;]
    Q_pp --> Mq_e2{are there messeges on queue?}
    Mq_e2 -->|Yes| Sid_send
    Mq_e2 -->|No| IL
    end
```