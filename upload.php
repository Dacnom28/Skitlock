<?php
// Directory to store decrypted files.
define('DECRYPTED_DIR', __DIR__ . '/decrypted_files');

// Make sure the decrypted directory exists.
if (!is_dir(DECRYPTED_DIR)) {
    mkdir(DECRYPTED_DIR, 0755, true);
}

// Function to decrypt files using AES-256-GCM.
function decryptFile($encryptedFile, $key, $iv) {
    // Read the contents of the encrypted file.
    $fileContents = file_get_contents($encryptedFile);
    if ($fileContents === false) {
        throw new Exception("Failed to read encrypted file.");
    }

    // GCM tag length (16 bytes)
    $tagLength = 16;
    $tag = substr($fileContents, -$tagLength);
    $ciphertext = substr($fileContents, 0, -$tagLength);

    // Decryption using OpenSSL
    $plaintext = openssl_decrypt(
        $ciphertext,
        'aes-256-gcm',
        $key,
        OPENSSL_RAW_DATA,
        $iv,
        $tag
    );

    if ($plaintext === false) {
        throw new Exception("Failed to decrypt file. Check key or IV..");
    }

    return $plaintext;
}

// Handle POST requests
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Check if file, key and IV were sent
    if (!isset($_FILES['file']) || !isset($_POST['key']) || !isset($_POST['iv'])) {
        http_response_code(400);
        echo "The file, key, or IV is incomplete.";
        exit;
    }

    // Save temporary files
    $uploadedFile = $_FILES['file']['tmp_name'];
    $originalFileName = $_FILES['file']['name'];

    // Receive key and IV from request
    $key = hex2bin($_POST['key']);
    $iv = hex2bin($_POST['iv']);

    if ($key === false || $iv === false) {
        http_response_code(400);
        echo "Invalid key or IV (must be in hex format).";
        exit;
    }

    try {
        // Dekripsi file
        $decryptedData = decryptFile($uploadedFile, $key, $iv);

        // Save the decrypted file
        $decryptedFilePath = DECRYPTED_DIR . '/' . $originalFileName . '.dec';
        file_put_contents($decryptedFilePath, $decryptedData);

        // Give a successful response
        http_response_code(200);
        echo "Files were successfully decrypted and saved to: $decryptedFilePath";
    } catch (Exception $e) {
        http_response_code(500);
        echo "Error:: " . $e->getMessage();
    }
} else {
    http_response_code(405);
    echo "Only POST method is supported.";
}
