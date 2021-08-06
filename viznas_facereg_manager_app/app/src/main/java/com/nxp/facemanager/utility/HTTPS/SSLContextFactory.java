package com.nxp.facemanager.utility.HTTPS;

import android.content.Context;
import android.util.Base64;

import com.nxp.facemanager.R;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.security.KeyStore;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;

/**
 * A factory for SSLContexts.
 * Builds an SSLContext with custom KeyStore and TrustStore, to work with a client cert signed by a self-signed CA cert.
 */
public class SSLContextFactory {

    private static final String X_509_FORMAT = "X509";
    private static final String SERVER_FILE_FORMAT = "PKCS12";
    private static SSLContextFactory theInstance = null;
    /**
     * SSL server password
     */
    private static final String SERVER_P12_PASSWORD = "nxp123";

    private SSLContextFactory() {
    }

    public static SSLContextFactory getInstance() {
        if(theInstance == null) {
            theInstance = new SSLContextFactory();
        }
        return theInstance;
    }

    /**
     * Creates an SSLContext with the client and server certificates
     * @param caCertString A String containing the server certificate
     * @return An initialized SSLContext
     * @throws Exception
     */
    public SSLContext makeContext(Context context, String caCertString) throws Exception {
        final KeyStore keyStore = loadPKCS12KeyStore(context);
        KeyManagerFactory kmf = KeyManagerFactory.getInstance(X_509_FORMAT);
        kmf.init(keyStore, SERVER_P12_PASSWORD.toCharArray());
        KeyManager[] keyManagers = kmf.getKeyManagers();

        final KeyStore trustStore = loadPEMTrustStore(caCertString);
        TrustManager[] trustManagers = {new CustomTrustManager(trustStore)};

        SSLContext sslContext = SSLContext.getInstance("TLSv1");
        sslContext.init(keyManagers, trustManagers, null);

        return sslContext;
    }

    /**
     * Produces a KeyStore from a String containing a PEM certificate (typically, the server's CA certificate)
     * @param certificateString A String containing the PEM-encoded certificate
     * @return a KeyStore (to be used as a trust store) that contains the certificate
     * @throws Exception
     */
    private KeyStore loadPEMTrustStore(String certificateString) throws Exception {

        byte[] der = loadPemCertificate(new ByteArrayInputStream(certificateString.getBytes()));
        ByteArrayInputStream derInputStream = new ByteArrayInputStream(der);
        CertificateFactory certificateFactory = CertificateFactory.getInstance(X_509_FORMAT);
        X509Certificate cert = (X509Certificate) certificateFactory.generateCertificate(derInputStream);
        String alias = cert.getSubjectX500Principal().getName();

        KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
        trustStore.load(null);
        trustStore.setCertificateEntry(alias, cert);

        return trustStore;
    }

    /**
     * Produces a KeyStore from a PKCS12 (.p12) certificate file, typically the client certificate
     * @return A KeyStore containing the certificate from the certificateFile
     * @throws Exception
     */
    private KeyStore loadPKCS12KeyStore(Context context) throws Exception {
        KeyStore keyStore = null;
        FileInputStream fis = null;
        try {
            keyStore = KeyStore.getInstance(SERVER_FILE_FORMAT);
            InputStream inputStream = context.getResources().openRawResource(R.raw.server);
            keyStore.load(inputStream, SERVER_P12_PASSWORD.toCharArray());
        } finally {
            try {
                if(fis != null) {
                    fis.close();
                }
            } catch(IOException ex) {
                // ignore
            }
        }
        return keyStore;
    }

    /**
     * Reads and decodes a base-64 encoded DER certificate (a .pem certificate), typically the server's CA cert.
     * @param certificateStream an InputStream from which to read the cert
     * @return a byte[] containing the decoded certificate
     * @throws IOException
     */
    byte[] loadPemCertificate(InputStream certificateStream) throws IOException {

        byte[] der = null;
        BufferedReader br = null;

        try {
            StringBuilder buf = new StringBuilder();
            br = new BufferedReader(new InputStreamReader(certificateStream));

            String line = br.readLine();
            while(line != null) {
                if(!line.startsWith("--")){
                    buf.append(line);
                }
                line = br.readLine();
            }

            String pem = buf.toString();
            der = Base64.decode(pem, Base64.DEFAULT);

        } finally {
           if(br != null) {
               br.close();
           }
        }

        return der;
    }
}
