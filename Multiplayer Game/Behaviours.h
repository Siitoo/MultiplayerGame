#pragma once


struct Behaviour
{
	GameObject *gameObject = nullptr;

	virtual void start() { }

	virtual void update() { }

	virtual void onInput(const InputController &input, bool server = true) { }
	virtual void onFakeInput(const InputController &input, float& angle, vec2& position) { }
	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void DoUpdate() { }
	virtual bool GetUpdate() { return false; }
};

struct Spaceship : public Behaviour
{
	void start() override
	{
		//srand(time(0));
		gameObject->parent_tag = (uint32)(Random.next() * UINT_MAX);
		//gameObject->position.x = rand() %2000 + 1000;
		//gameObject->position.y = rand() % 2000 + 1000;
	}

	void onInput(const InputController &input, bool server) override
	{
		if (input.horizontalAxis != 0.0f)
		{
			const float rotateSpeed = 180.0f;
			gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;
			if(server)
				NetworkUpdate(gameObject);
		}

		if (input.actionDown == ButtonState::Pressed)
		{
			const float advanceSpeed = 200.0f;
			gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;
			if(server)
				NetworkUpdate(gameObject);
		}

		if (input.actionLeft == ButtonState::Press && server)
		{
			GameObject * laser = App->modNetServer->spawnBullet(gameObject);
			laser->parent_tag = gameObject->parent_tag;
		}
	}

	void onFakeInput(const InputController &input, float& angle, vec2& position)
	{
		if (angle != 0)
		{
			const float rotateSpeed = 180.0f;
			angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;
		}
		

		if(position.x != 0 || position.y != 0)
		{
			if (Input.actionDown == Pressed)
			{
				const float advanceSpeed = 200.0f;
				position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;
			}

		}

	}

	void onCollisionTriggered(Collider &c1, Collider &c2) override
	{
		if (c2.type == ColliderType::Laser && c2.gameObject->parent_tag != c1.gameObject->parent_tag)
		{
			if (c2.gameObject->behaviour->GetUpdate()) // Check if are a server testing collision
			{
				NetworkDestroy(c2.gameObject); // Destroy the laser

				if (c1.gameObject->totalLife  == 1)
				{
					NetworkDestroy(c1.gameObject);
					c2.gameObject->parent->totalKills++;
				}
				else
					c1.gameObject->totalLife--;


				// NOTE(jesus): spaceship was collided by a laser
				// Be careful, if you do NetworkDestroy(gameObject) directly,
				// the client proxy will poing to an invalid gameObject...
				// instead, make the gameObject invisible or disconnect the client.
			}
		}
	}
};

struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;
	float secondsSinceCreationClient = 0.0f;
	bool do_update = false;

	void DoUpdate()
	{
		do_update = true;
	}

	void update() override
	{
		const float pixelsPerSecond = 1000.0f;
		if (do_update)
		{
			gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

			secondsSinceCreation += Time.deltaTime;

			NetworkUpdate(gameObject);
		}
		else
		{
	
			secondsSinceCreationClient += Time.deltaTime;
			const float lifetimeSecondsClient = 10.0f;
			if (secondsSinceCreationClient > lifetimeSecondsClient)
			{
				App->modLinkingContext->unregisterNetworkGameObject(gameObject);
				if(gameObject->networkId == 0)
					Destroy(gameObject);
			}
		}
		const float lifetimeSeconds = 2.0f;

		if (secondsSinceCreation > lifetimeSeconds) NetworkDestroy(gameObject);
	}

	bool GetUpdate() { return do_update; }

};
