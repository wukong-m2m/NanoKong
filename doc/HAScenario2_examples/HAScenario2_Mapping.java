public class HAScenario2_Mapping {
  private final static byte[] componentInstanceToEndpointMap = { // Indexed by component instance id.
    (byte)1, (byte)0x1, // Component 0: input controller    @ node 1, port 1
    (byte)1, (byte)0x2, // Component 1: light sensor        @ node 1, port 2
    (byte)3, (byte)0x3, // Component 2: threshold           @ node 3, port 3
    (byte)3, (byte)0x4, // Component 3: light               @ node 3, port 4
    (byte)1, (byte)0x5, // Component 4: occupancy           @ node 1, port 5
    (byte)3, (byte)0x5  // Component 5: and gate            @ node 3, port 5
  };
}
