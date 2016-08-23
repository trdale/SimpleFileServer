import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.ServerSocket;

public class ftclient {

	public final static int FILE_SIZE = 12096;
	
	public static void main (String [] args ) throws IOException {
		int bytesRead;
		int current = 0;
		FileOutputStream fos = null;
		BufferedOutputStream bos = null;
		OutputStream os = null;
		Socket datasock = null;
		Socket mainsock = null;
		ServerSocket servsock = null;
		String hostname = null;
		String command = null;
		String filename = null;	 
		String msg = null;
		int servPort = 0;
		int dataPort = 0;
		
		if(args.length < 4 || args.length > 5){
			System.out.println("Usage: ftclient.java <server host> <server port> -g|-l <filename> <data port>");
			System.exit(1);
		}
		
		hostname = args[0];
		servPort = Integer.parseInt(args[1]);
		command = args[2];
		if (command.equals("-g")) {
			filename = args[3];
			dataPort = Integer.parseInt(args[4]);
		}
		else if (command.equals("-l")) {
			filename = null;
			dataPort = Integer.parseInt(args[3]);		 
		}  

		try {
			//set up connections
			System.out.printf("Open on data port %d\n", dataPort);
			servsock = new ServerSocket(dataPort);
			mainsock = new Socket(hostname, servPort);
			System.out.printf("Connected to host %s on port %d\n", hostname, servPort);

			//send message
			msg = command + " " + hostname + " " + dataPort + " " + filename;
			os = mainsock.getOutputStream();
			os.write(msg.getBytes());
			os.flush();

			//accept connection
			datasock = servsock.accept();

			if (command.equals("-g")){
				//recieve the file
				byte [] bytearray = new byte [FILE_SIZE];
				InputStream is = datasock.getInputStream();
				fos = new FileOutputStream(filename + ".copy");
				bos = new BufferedOutputStream(fos);
				
				bytesRead = is.read(bytearray, 0, bytearray.length);
				current = bytesRead;

				do {
					bytesRead = is.read(bytearray, current, (bytearray.length-current));
					if(bytesRead >= 0) {
						current += bytesRead;
					}   

				} while (bytesRead > -1);
	
				bos.write(bytearray, 0, current);
				bos.flush();
				System.out.println("File " + filename + " transfered (" + current + " bytes read)");
			
			} else {
				byte [] bytearray = new byte [FILE_SIZE];
				InputStream is = datasock.getInputStream();
				
				bytesRead = is.read(bytearray, 0, bytearray.length);
				current = bytesRead;
				
				do {
					bytesRead = is.read(bytearray, current, (bytearray.length-current));
					if(bytesRead >= 0) {
						current += bytesRead;
					}
				} while(bytesRead > -1); 

				System.out.write(bytearray);
			}			 	
		}
		finally {
			if (fos != null) fos.close();
			if (bos != null) bos.close();
			if (servsock != null) servsock.close();
			if (datasock != null) datasock.close();
			if (mainsock != null) mainsock.close();				
		}	
	}	
}	
