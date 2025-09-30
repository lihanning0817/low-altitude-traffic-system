import Anthropic from "@anthropic-ai/sdk";

const client = new Anthropic({
  apiKey: process.env.ANTHROPIC_API_KEY,
});

async function main() {
  const msg = await client.messages.create({
    model: "claude-3-5-haiku-20241022",
    max_tokens: 100,
    messages: [{ role: "user", content: "你好，Claude!" }],
  });

  console.log(msg);
}

main().catch(console.error);
