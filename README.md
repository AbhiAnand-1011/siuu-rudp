# SIUU-RUDP

SIUU-RUDP is a C++ implementation of a **reliable file transfer protocol built on top of UDP**.  
The project demonstrates how reliability, ordering, and data integrity can be implemented at the application layer without relying on TCP.

---

## Motivation

UDP is fast and lightweight but does not guarantee:
- reliable delivery
- packet ordering
- duplicate suppression
- end-to-end integrity

SIUU-RUDP builds these guarantees **on top of UDP**, illustrating how transport-layer mechanisms such as sequencing, acknowledgements, and checksums work internally.

---

## High-Level Design

The system follows a **sender–receiver architecture** over UDP:

- **Sender**
  - Reads a file from disk
  - Splits it into fixed-size chunks
  - Sends chunks as sequenced UDP packets
  - Computes and sends integrity metadata

- **Receiver**
  - Receives UDP packets (possibly out of order)
  - Verifies payload integrity
  - Stores chunks temporarily
  - Reassembles the original file once all chunks arrive

Both sides communicate using a **custom application-layer protocol** defined in shared protocol files.

---

## Architecture Overview

