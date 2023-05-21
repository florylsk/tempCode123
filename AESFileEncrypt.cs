  public class AesFileEncryptionServices
    {
        private const int BufferSizeInBytes = 16 * 1024;

        // you can have a number of other lengths but these will do
        private const int RequiredKeyLengthInBytes = 32;
        private const int RequiredInitialisationVectorLengthInBytes = 16;

        public void EncryptFile(string inputFile, string outputFile, string key, string initialisationVector)
        {
            using (RijndaelManaged aes = new RijndaelManaged())
            {
                byte[] keyBytes = ASCIIEncoding.UTF8.GetBytes(key);
                byte[] initialisationVectorBytes = ASCIIEncoding.UTF8.GetBytes(initialisationVector);

                this.ValidateKey(keyBytes);
                this.ValidateInitialisation(initialisationVectorBytes);

                using (FileStream outputFileStream = new FileStream(outputFile, FileMode.Create))
                {
                    using (ICryptoTransform encryptor = aes.CreateEncryptor(keyBytes, initialisationVectorBytes))
                    {
                        using (CryptoStream outputCryptoStream = new CryptoStream(outputFileStream, encryptor, CryptoStreamMode.Write))
                        {
                            using (FileStream inputFileStream = new FileStream(inputFile, FileMode.Open))
                            {
                                byte[] buffer = new byte[BufferSizeInBytes];
                                int count;
                                while ((count = inputFileStream.Read(buffer, 0, buffer.Length)) != 0)
                                {
                                    outputCryptoStream.Write(buffer, 0, count);
                                }
                            }
                        }
                    }
                }
            }
        }

        public void DecryptFile(string inputFile, string outputFile, string key, string initialisationVector)
        {
            using (RijndaelManaged aes = new RijndaelManaged())
            {
                byte[] keyBytes = ASCIIEncoding.UTF8.GetBytes(key);
                byte[] initialisationVectorBytes = ASCIIEncoding.UTF8.GetBytes(initialisationVector);

                this.ValidateKey(keyBytes);
                this.ValidateInitialisation(initialisationVectorBytes);

                using (FileStream inputFileStream = new FileStream(inputFile, FileMode.Open))
                {
                    using (FileStream outputFileStream = new FileStream(outputFile, FileMode.Create))
                    {
                        using (ICryptoTransform decryptor = aes.CreateDecryptor(keyBytes, initialisationVectorBytes))
                        {
                            using (CryptoStream inputCryptoStream = new CryptoStream(inputFileStream, decryptor, CryptoStreamMode.Read))
                            {
                                byte[] buffer = new byte[BufferSizeInBytes];
                                int count;
                                while ((count = inputCryptoStream.Read(buffer, 0, buffer.Length)) != 0)
                                {
                                    outputFileStream.Write(buffer, 0, count);
                                }
                            }
                        }
                    }
                }
            }
        }

        private void ValidateKey(byte[] key)
        {
            if (key.Length == RequiredKeyLengthInBytes)
            {
                return;
            }
            throw new ArgumentException(string.Format("Please use a key length of {0} bytes ({1} bits)", RequiredKeyLengthInBytes, RequiredKeyLengthInBytes * 8));
        }

        private void ValidateInitialisation(byte[] initialisationVector)
        {
            if (initialisationVector.Length == RequiredInitialisationVectorLengthInBytes)
            {
                return;
            }
            throw new ArgumentException(string.Format("Please use an initialisation vector length of {0} bytes ({1} bits)", RequiredInitialisationVectorLengthInBytes, RequiredInitialisationVectorLengthInBytes * 8));
        }
    }
