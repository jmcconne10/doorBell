import discord
import firebase_admin
from firebase_admin import credentials, db
import os
import asyncio
from dotenv import load_dotenv

setSleep = 10

# Load environment variables from .env file
load_dotenv()
TOKEN = os.getenv('DISCORD_BOT_TOKEN')

# Initialize Firebase with your service account credentials
cred = credentials.Certificate("serviceAccountKey.json")  # Update with the path to your JSON file
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://doorbell-338a5-default-rtdb.firebaseio.com/'
})

# Create intents object and enable necessary intents
intents = discord.Intents.default()
intents.message_content = True  # Required to read message content

client = discord.Client(intents=intents)

# Replace 'your_channel_id' with the actual ID of the channel
TARGET_CHANNEL_ID = 1306613834844344430  # Copy the channel ID here

# Function to update Firebase for Alert
def update_firebase_alert(value):
    ref = db.reference("ALERT")
    ref.set(value)  # Set the ALERT value to the provided value
    print(f"Firebase ALERT set to {value}")

def update_firebase_user(value):
    ref = db.reference("USER")
    if isinstance(value, (discord.Member, discord.User)):
        # Store only the user's name
        user_name = value.name
        ref.set(user_name)  # Store the name in Firebase
        print(f"Firebase USER set to {user_name}")
    else:
        # If value is not a user, store it directly
        ref.set(value)
        print(f"Firebase USER set to {value}")


@client.event
async def on_ready():
    print(f'We have logged in as {client.user}')

@client.event
async def on_message(message):
    # Check if the message is from the target channel
    if message.channel.id == TARGET_CHANNEL_ID:
        print(f"Message from {message.author}: {message.content}")
        if message.content == "!gate":
            print("Gate command received")
            update_firebase_alert(True)  # Update Firebase when !gate is received
            update_firebase_user(message.author)

            
            # Wait for 10 seconds, then reset the ALERT to False
            await asyncio.sleep(setSleep)
            update_firebase_alert(False)  # Reset ALERT to False after 10 seconds
            update_firebase_user("")

# Run the bot with your token
client.run(TOKEN)
