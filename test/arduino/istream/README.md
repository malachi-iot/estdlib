# Unusual but intended behaviors: arduino_istream

Does not wait for EOL on extraction operator, making it slightly more "interactive"

Does not echo back to console during extraction operator.
This might be an interesting option for the future, but for the moment this is correct