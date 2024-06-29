# Compile the P4 Program:
Ensure your P4 program (in this case, binary_adder.p4) is compiled. You can typically do this with a P4 compiler command like:


- p4c --target bmv2 --arch v1model --std p4-16 binary_adder.p4 -o binary_adder.json


# Prepare the Environment:
Make sure you have p4app and Mininet installed. You can use the p4app Docker container for a pre-configured environment.

# Run the P4 Application:
Start the P4 application using the p4app tool. Place p4app.json and binary_adder.p4 in the same directory, then run:

- p4app run


# Execute the Mininet Script:
Run the Mininet setup script to start the Mininet environment and the controller:


- sudo python3 mininet_setup.py


# Testing:
Use the Mininet CLI to test the functionality of the 4-bit binary adder by sending appropriate packets from h1 to h2 and observing the results.