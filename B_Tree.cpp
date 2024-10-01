#include<stdio.h>
#include<stdlib.h>
#include<math.h>

struct Node{
	int *val, curAmount, childAmount;
	struct Node **childs, *parent;
}*root;

void checkNode(struct Node *curr, int deletedPos);

int order = 3;
char c;

//function to make it easier to create a new node
//aka constructor
struct Node *createNode(int val){
	struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->val = (int*)malloc(sizeof(int) * order);
	newNode->val[0] = val;
	newNode->curAmount = 1;
	newNode->childAmount = 0;
	newNode->childs = (struct Node**)malloc(sizeof(struct Node*) * (order + 1));
	newNode->parent = NULL;
	
	return newNode;
}

//function to get the max value out of two value
int max(int a, int b){
	return a > b ? a : b;
}

//same thing but for min value
int min(int a, int b){
	return a < b ? a : b;
}

//convert uppercase letter to lowercase
char toLower(char let){
	if (let >= 65 && let <= 90) return let + 32;
	return let;
}

//wait for user input before continuing
void enterToContinue(){
	printf("Press enter to continue...\n");
	getchar();
}

//get the height of the tree, or just part of the tree if needed
int getHeight(struct Node *curRoot){
	//only need to get height of leftmost leaf since the depth of every leaf in a balanced B-tree is always the same
	if (!curRoot) return -1;
	else if (curRoot->childAmount == 0) return 0;
	else return getHeight(curRoot->childs[0]) + 1;
}

//print one specific level of the tree, aka specific depth
void printLevel(struct Node *curRoot, int level, int curLevel){
	//check if node exist, should only be called if the root is null
	if (curRoot){
		//if currently at the correct depth, print
		if (level == curLevel){
			printf("Level %d:", level);
			for (int i = 0; i < curRoot->curAmount; i++) printf(" %d", curRoot->val[i]);
			printf(" Child: %d", curRoot->childAmount);
			printf("\n");
		}
		//if not, go down further, do this for every child that the node have, recursion will stop when reaching leaf
		else{
			for (int i = 0; i < curRoot->childAmount; i++) printLevel(curRoot->childs[i], level, curLevel + 1);
		}
	}
}

//function to print every level of the tree, it basically call print level for every level that the tree have
void printAll(){
	system("cls");
	//to check if the tree exist or not
	if (root){
		//first get the height, then print each level/depth
		int treeHeight = getHeight(root);
		for (int i = 0; i <= treeHeight; i++){
			printLevel(root, i, 0);
		}
	}
	else{
		printf("B-Tree is empty!\n");
	}
	
	enterToContinue();
}

//add a new value to an existing node
void addValue(struct Node *curr, int newVal, int pos){
	//rearrange values in the node to give space to the new value, only called if the new pos isn't at the end
	for (int i = curr->curAmount - 1; i >= pos; i--){
		curr->val[i + 1] = curr->val[i];
	}
	//after rearranging, put the new value in the pos
	curr->val[pos] = newVal;
	curr->curAmount++;
}

//version without giving specific pos
void addValueNoPos(struct Node *curr, int newVal){
	//could use binary search, probably, unsure
	for (int i = 0; i < curr->curAmount; i++){
		//first it will find the position of the new value
		if (newVal <= curr->val[i]){
			//after finding the correct position, it will then rearrange the values inside the node as usual
			for (int j = curr->curAmount - 1; j >= i; j--){
				curr->val[j + 1] = curr->val[j];
			}
			curr->val[i] = newVal;
			curr->curAmount++;
			return;
		}
	}
	//if num bigger than all val in node
	//if bigger than it will just insert it into the end of the node so dont need to rearrange
	curr->val[curr->curAmount] = newVal;
	curr->curAmount++;
}

//remove a value from an existing node
void removeValue(struct Node *curr, int pos){
	if (pos < curr->curAmount){
		for (int i = pos + 1; i < curr->curAmount; i++){
			curr->val[i - 1] = curr->val[i];
		}
	}
	curr->curAmount--;
}

//add a new child to an existing node
void addChild(struct Node *parent, struct Node *child, int pos){
	//rearrange the childs to give way to the new child
	for (int i = parent->childAmount - 1; i >= pos; i--){
		parent->childs[i + 1] = parent->childs[i];
	}
	//put the new child to the parent
	parent->childs[pos] = child;
	child->parent = parent;
	parent->childAmount++;
}

//switch the parent of a child to another parent
void stealChild(struct Node *newParent, struct Node *oldParent, int childOldPos, int childNewPos){
	//rearrange the childs on the new parent to give way to the new child
	for (int i = newParent->childAmount - 1; i >= childNewPos; i--){
		newParent->childs[i + 1] = newParent->childs[i];
	}
	//switch parent
	newParent->childs[childNewPos] = oldParent->childs[childOldPos];
	newParent->childs[childNewPos]->parent = newParent;
	newParent->childAmount++;
	oldParent->childAmount--;
	
	//rearrange the childs on the old parent so there isnt an empty spot
	for (int i = childOldPos + 1; i <= oldParent->childAmount; i++){
		oldParent->childs[i - 1] = oldParent->childs[i];
	}
	//probs not necessasry
	//EDIT: absolutely not necessary since I never check using null, only using childAmount, but still good practice to null it
	oldParent->childs[oldParent->childAmount] = NULL;
}

//completely delete a child from a parent
void removeChild(struct Node *parent, struct Node *nodeToRemove, int nodePosInParent){
	//free the memory for the pointer
	free(nodeToRemove->val);
	//might completely remove their childs altogether or might only remove the pointer that access them, hoping for the latter
	//EDIT: does the latter, works perfectly fine
	free(nodeToRemove->childs);
	free(nodeToRemove);
	parent->childAmount--;
	//rearrange, might be able to put them into their own separate function at this point since I used them so much
	for (int i = nodePosInParent + 1; i <= parent->childAmount; i++){
		parent->childs[i - 1] = parent->childs[i];
	}
	//considering my conditions use childamount instead of checking whether a node is null or not
	//this is most likely not necessary, pretty certain actually
	parent->childs[parent->childAmount] = NULL;
	//almost certainly not necessary
	nodeToRemove = NULL;
}

//split a node into 3 parts, the parent and 2 childs
void splitNode(struct Node *curr){
	//get the mid position, which will be the value for the parent
	int mid = ceil(order / 2.0) - 1;
	//the right child for the parent, for the left it will just use the curr node
	struct Node *newRightChild = createNode(curr->val[mid + 1]);
	//add the values after mid to the new right node, why + 2 you ask? i dont actually remember but it works
	//EDIT: + 2 since the first value is already added when we create the right child
	for (int i = mid + 2; i < order; i++){
		//for pos value might be able to just use the curAmount on the newRightChild but it might take more time idk
		addValue(newRightChild, curr->val[i], i - mid - 1);
	}
	//if it already have a parent, then just add the mid value to the parent, else create a new parent
	if (curr->parent){
		//might be able to improve on this checking, a lot of codes in this file can be improved probably
		//used to find the position of the curr node on the parent
		for (int i = 0; i < order; i++){
			if (curr->parent->childs[i] == curr){
				addValue(curr->parent, curr->val[mid], i);
				//might not be needed, changing their value to 0 doesnt do much anyway
				for (int j = mid; j < order; j++){
					curr->val[j] = 0;
				}
				curr->curAmount = mid;
				
				addChild(curr->parent, newRightChild, i + 1);
				//if after adding the new value, the parent also reaches above max capacity, then split again but for the parent
				if (curr->parent->curAmount == order) splitNode(curr->parent);
				break;
			}
		}
	}
	//if parent doesnt exist yet, then create a new parent
	else{
		struct Node *newParent = createNode(curr->val[mid]);
		
		//might not be needed, changing their value to 0 doesnt do much anyway
		for (int i = mid; i < order; i++){
			curr->val[i] = 0;
		}
		curr->curAmount = mid;
		//set relations
		curr->parent = newParent;
		newRightChild->parent = newParent;
		//can just use 0, 1, 2 since when we first create the parent, the two childs will always be the first and second child
		newParent->childs[0] = curr;
		newParent->childs[1] = newRightChild;
		newParent->childAmount = 2;
		//only way parent doesnt exist yet is if the current node is the root, so when we add a new parent, then the new parent should become root
		root = newParent;
	}
	
	//split the ownership of the childs, only for internal nodes, when internal nodes split, their childs is also split into two parts
	//one part staying with the left node one part switching to the right
	mid = curr->childAmount / 2;
	for (int i = mid; i < curr->childAmount; i++){
		//now that i think about it, could probably use stealchild instead, not gonna mess with the code though, if it works it works
		//also, this is probably better, it's most likely faster by a very very small margin
		//also also, doesnt need to worry about memory and stuff since addchild doesnt create a new child, only assign a node that
		//you created to be a child of a node of your choosing
		addChild(newRightChild, curr->childs[i], i - mid);
		//not necessary but still good practice, make sure that the old parent wont be able to access the childs
		curr->childs[i] = NULL;
	}
	curr->childAmount = mid;
}

//merge two leaf childs to be one
void mergeChilds(struct Node *mergeTo, struct Node *secondChild, int posInParent, int secondPosInParent){
	//add values from one child to another, doesnt use pos in here since it's possible to take values from the left or right child
	//so assuming a position would cause problem, creating a separate function for both merge would make it harder to maintain
	for (int i = 0; i < secondChild->curAmount; i++){
		addValueNoPos(mergeTo, secondChild->val[i]);
	}
	
	//when merging, the node will take a value from the parent
	struct Node *parent = mergeTo->parent;
	//it will take a value that is in between both childs, which conveniently can be gotten using the left node pos in the parent, which is the min pos
	int minPos = min(posInParent, secondPosInParent);
	addValueNoPos(mergeTo, parent->val[minPos]);
	removeValue(parent, minPos);
	removeChild(parent, secondChild, secondPosInParent);
	//check the parent node, since it's possible that after donating a value, the parent itself need a new value
	checkNode(parent, minPos);
}

//same thing but for internal nodes, since internal nodes need to switch ownership of childs as well
void mergeInternalNodes(struct Node *mergeTo, struct Node *mergeFrom, int posInParent, int secondPosInParent, int siblingInRight){
	//save to variable so it doesnt get changed during stealing
	int siblingChildAmount = mergeFrom->childAmount;
	for (int i = 0; i < siblingChildAmount; i++){
		//the position to put the new child differ depending on whether it's merging with its left sibling or right sibling
		if (siblingInRight){
			stealChild(mergeTo, mergeFrom, 0, mergeTo->childAmount);
		}
		else{
			stealChild(mergeTo, mergeFrom, mergeFrom->childAmount - 1, 0);
		}	
	}
	
	//after stealing their sibling's childs, it will steal all of their values
	for (int i = 0; i < mergeFrom->curAmount; i++){
		addValueNoPos(mergeTo, mergeFrom->val[i]);
	}
	struct Node *parent = mergeTo->parent;
	int minPos = min(posInParent, secondPosInParent);
	//then it will steal a value from its parent
	addValueNoPos(mergeTo, parent->val[minPos]);
	removeValue(parent, minPos);
	removeChild(parent, mergeFrom, secondPosInParent);
	//check its parent as usual
	checkNode(parent, minPos);
}

//check whether a node need borrowing or merging
void checkNode(struct Node *curr, int deletedPos){
	//get the minimum keys/values a node must have
	int minimumKeys = ceil(order / 2.0) - 1;
	//if leaf and doesnt violate minimum keys or if the only node left(root)
	if (curr->childAmount == 0 && (curr->curAmount >= minimumKeys || curr->parent == NULL)) return;
	//if violate minimum keys or internal node
	//if leaf
	if (curr->childAmount == 0 && curr->parent != NULL){
		//remove current value that want to be deleted
		struct Node *parent = curr->parent;
		int currPosInParent;
		//get the curr node position in the parent, used to check its left and right sibling
		for (int i = 0; i < parent->childAmount; i++){
			if (parent->childs[i] == curr){
				currPosInParent = i;
				break;
			}
		}
		//have left sibling and left sibling have more than minimum keys
		if (currPosInParent > 0 && parent->childs[currPosInParent - 1]->curAmount > minimumKeys){
			struct Node *leftSibling = parent->childs[currPosInParent - 1];
			//add a key from parent to current
			addValueNoPos(curr, parent->val[currPosInParent - 1]);
			//remove that key from the parent
			removeValue(parent, currPosInParent - 1);
			//add last key from left sibling to parent
			addValueNoPos(parent, leftSibling->val[leftSibling->curAmount - 1]);
			//remove that key from left sibling
			removeValue(leftSibling, leftSibling->curAmount - 1);
		}
		//have right sibling and right sibling have more than minimum keys
		else if (currPosInParent + 1 < parent->childAmount && parent->childs[currPosInParent + 1]->curAmount > minimumKeys){
			struct Node *rightSibling = parent->childs[currPosInParent + 1];
			//might have a way to get the pos, pretty sure there is a way, couldnt bother thinking atm, might do later
			addValueNoPos(curr, parent->val[currPosInParent]);
			removeValue(parent, currPosInParent);
			addValueNoPos(parent, rightSibling->val[0]);
			removeValue(rightSibling, 0);
		}
		//siblings have less than minimum keys
		else{
			//first, merge with left, if not possible then try right
			if (currPosInParent > 0){
				mergeChilds(curr, parent->childs[currPosInParent - 1], currPosInParent, currPosInParent - 1);
			}
			else{
				mergeChilds(curr, parent->childs[currPosInParent + 1], currPosInParent, currPosInParent + 1);
			}
		}
	}
	//if internal node
	else{
		//need to borrow or merge, root cannot borrow from its sibling since root doesnt have one, also cant merge since no siblings
		if (curr->childAmount < ceil(order / 2.0) && curr != root){
			//similar to borrow and merge of leaf nodes, but this time it will also steal a child from its sibling
			struct Node *parent = curr->parent;
			int currPosInParent;
			for (int i = 0; i < parent->childAmount; i++){
				if (parent->childs[i] == curr){
					currPosInParent = i;
					break;
				}
			}
			//have left sibling and left sibling have more than minimum keys
			if (currPosInParent > 0 && parent->childs[currPosInParent - 1]->curAmount > minimumKeys){
				struct Node *leftSibling = parent->childs[currPosInParent - 1];
				//add a key from parent to current
				addValueNoPos(curr, parent->val[currPosInParent - 1]);
				//remove that key from the parent
				removeValue(parent, currPosInParent - 1);
				//add last key from left sibling to parent
				addValueNoPos(parent, leftSibling->val[leftSibling->curAmount - 1]);
				//remove that key from left sibling
				removeValue(leftSibling, leftSibling->curAmount - 1);
				//steal child from left sibling
				//pretty sure if we can borrow from left sibling, that means it have more than minimun childs so dont need to check
				stealChild(curr, leftSibling, leftSibling->childAmount - 1, 0);
			}
			//have right sibling and right sibling have more than minimum keys
			else if (currPosInParent + 1 < parent->childAmount && parent->childs[currPosInParent + 1]->curAmount > minimumKeys){
				struct Node *rightSibling = parent->childs[currPosInParent + 1];
				addValueNoPos(curr, parent->val[currPosInParent]);
				removeValue(parent, currPosInParent);
				addValueNoPos(parent, rightSibling->val[0]);
				removeValue(rightSibling, 0);
				stealChild(curr, rightSibling, 0, curr->childAmount);
			}
			//siblings have less than minimum keys
			else{
				//first, merge with left, if not possible then try right
				if (currPosInParent > 0){
					mergeInternalNodes(curr, parent->childs[currPosInParent - 1], currPosInParent, currPosInParent - 1, 0);
				}
				else{
					mergeInternalNodes(curr, parent->childs[currPosInParent + 1], currPosInParent, currPosInParent + 1, 1);
				}
			}
		}
		//an internal nodes at all times must have n amount of keys/values and n + 1 amount of childs
		//so if it doesnt meet that requirement, that mean it needs to get a replacement, happen when a value from an internal node is deleted
		//could probably change the condition to !=, but it can only be n keys and n + 1 childs or n - 1 keys and n + 1 childs anyway so wont bother
		else if (curr->curAmount < curr->childAmount - 1){
			//can choose to get replacement from left or right, in here we use left
			//get rightmost of left child, basically same thing as getting replacement in a binary tree
			struct Node *child = curr->childs[deletedPos];
			while(child->childAmount > 0){
				child = child->childs[child->childAmount - 1];
			}
			//after getting the child, then it will steal it's rightmost value(biggest value), then check whether the child needs to be rearranged or not
			addValueNoPos(curr, child->val[child->curAmount - 1]);
			removeValue(child, child->curAmount - 1);
			checkNode(child, child->curAmount);
		}
		//can only happen if the root only have one value and the child merge, taking that one value away
		//if you were to simply delete the value on the root while it still have two childs, then the if above would be called instead
		else if (root && curr->curAmount == 0){
			//if this happen, root should only have one child, probs idk
			root = root->childs[0];
			free(root->parent->val);
			free(root->parent->childs);
			free(root->parent);
			root->parent = NULL;
		}
	}
}

//function to delete a value from a node, accidentally called it deletenode instead of deleteval
//gone too far, wouldnt dare changing it anymore tbh
void deleteNode(int val){
	struct Node *curr = root;
	//value to know whether the value that we want to delete has been found
	//value moved to know whether the curr node have went down to its child or not
	//if it havent and still have child, that means it's bigger than the biggest value so it should go to the rightmost child
	int foundPos = -1, moved = 0;
	while(curr && foundPos == -1){
		moved = 0;
		//most of these for loop can probably be changed to binary search
		//basically to navigate to the value that we want to find, similar to search in bst
		for (int i = 0; i < curr->curAmount; i++){
			if (val < curr->val[i] && curr->childAmount > i){
				moved = 1;
				curr = curr->childs[i];
				break;
			}
			else if (val == curr->val[i]){
				foundPos = i;
				break;
			}
		}
		//if value is bigger than the biggest val in node (go to rightmost child)
		if (!moved && foundPos == -1 && curr->childAmount > 0){
			curr = curr->childs[curr->curAmount];
		}
		//if it havent moved yet, havent found the value, and yet doesnt have any child(leaf node)
		//that mean the value is not in the tree, so we should stop searching
		else if (!moved && foundPos == -1 && curr->childAmount == 0){
			break;
		}
	}
	
	//if the value is found
	if (foundPos != -1){
		//delete the value
		removeValue(curr, foundPos);
		//if the value is the last value left in the tree, then just delete the entire tree
		if (curr == root && root->childAmount == 0 && root->curAmount == 0){
			free(root->val);
			free(root->childs);
			free(root);
			root = NULL;
		}		
		//if not, then check do we need to rearrange the tree or not after deletion
		else checkNode(curr, foundPos);
		printf("Successfully deleted %d from the tree\n", val);
	}
	else printf("%d is not in tree!\n", val);
}

//function to insert a new value into the tree
void insert(int val){
	//if the tree isnt created yet, then create it
	if (!root) root = createNode(val);
	else{
		struct Node *curr = root;
		int finished = 0;
		//basically, while we still havent found the correct position to put the value in, keep navigating through the tree
		while(!finished){
			//new value can only be inserted into a leaf node
			if (curr->childAmount == 0){
				//could use binary search to speed things up IF each node contains like 1000 value or something
				for (int i = 0; i < order; i++){
					if (curr->curAmount <= i || val <= curr->val[i]){
						addValue(curr, val, i);
						finished = 1;
						break;
					}
				}
			}
			else{
				for (int i = 0; i < order; i++){
					//no need to check whether curr val is null or not, handled by above if, pretty sure at least
					//above comment doesnt mean anything, might keep it around though
					//pretty sure doesnt need to check whether child is null or not
					if (curr->curAmount <= i || val <= curr->val[i]){
						curr = curr->childs[i];
						break;
					}
				}	
			}		
		}
		//if the node have more than the max amount of keys, then split it
		if (curr->curAmount == order) splitNode(curr);
	}
}

//completely delete every node and free every memory of the tree
//the return value is purely to we can set the root back to null
//not gonna do it outside the function since it might cause some error if we dont pay attention
struct Node *clearTree(struct Node *curRoot){
	if (!curRoot) return NULL;
	else if (curRoot->childAmount > 0){
		for (int i = 0; i < curRoot->childAmount; i++){
			clearTree(curRoot->childs[i]);
		}
	}
	free(curRoot->val);
	free(curRoot->childs);
	free(curRoot);
	return NULL;
}

//the menu to insert a new value
void insertNewValue(){
	system("cls");
	//if user input something else than numbers, it will just input 0 into the tree, or if a number have been inserted before,
	//it will just insert the last number again
	int newVal = 0, amount = 0;
	printf("How many numbers do you want to insert?\n");
	scanf("%d", &amount);
	while((c = getchar()) != EOF && c != '\n'){}
	for (int i = 1; i <= amount; i++){
		printf("Insert value %d: ", i);
		scanf("%d", &newVal);
		while((c = getchar()) != EOF && c != '\n'){}
		insert(newVal);
		printf("Successfully inserted %d into the tree\n", newVal);
	}
	enterToContinue();
}

//the menu to change the order value of the tree
void changeOrder(){
	char choice = 'a';
	do{
		system("cls");
		printf("Current Order: %d\n"
		"Are you sure you want to change the order? This will reset the tree and remove every node (Y/N)\n", order);
		scanf("%c", &choice);
		while((c = getchar()) != EOF && c != '\n'){}
		choice = toLower(choice);
	}while(choice != 'y' && choice != 'n');
	if (choice == 'y'){
		root = clearTree(root);
		order = 0;
		do{
			printf("Insert new order (>= 3): ");
			scanf("%d", &order);
			while((c = getchar()) != EOF && c != '\n'){}
			if (order < 3) printf("Order must be at least 3!\n");
		}while (order < 3);	
		printf("New order is %d\n", order);
		enterToContinue();
	}
}

//function to print the details of a specific value
void printValue(struct Node *curr, int key, int curDepth){
	if (!curr) {
		printf("Value is not in tree!\n");
		return;
	}
	//use recursion to print
	for (int i = 0; i < curr->curAmount; i++){
		if (key < curr->val[i] && curr->childAmount != 0){
			printValue(curr->childs[i], key, curDepth + 1);
			return;
		}
		else if (key == curr->val[i]){
			printf("Value: %d\n"
			"Position in node: %d\n"
			"Amount of value in node: %d\n"
			"Amount of childs: %d\n"
			"Node depth: %d\n", key, i, curr->curAmount, curr->childAmount, curDepth);
			printf("Value in node:");
			for (int j = 0; j < curr->curAmount; j++) printf(" %d", curr->val[j]);
			printf("\n");
			printf("Value in parent:");
			if (curr->parent){
				for (int j = 0; j < curr->parent->curAmount; j++) printf(" %d", curr->parent->val[j]);
				printf("\n");
			}
			else printf(" Node is root\n");
			return;
		}
	}
	if (curr->childAmount != 0) printValue(curr->childs[curr->childAmount - 1], key, curDepth + 1);
	else printf("Value is not in tree!\n");
}

//menu to find a value
void findAValue(){
	int key = 0;
	system("cls");
	printf("Insert value that you want to search: ");
	scanf("%d", &key);
	while((c = getchar()) != EOF && c != '\n'){}
	system("cls");
	printValue(root, key, 0);
	enterToContinue();
}

//menu to delete a value
void deleteAValue(){
	//default is 0
	int key = 0;
	system("cls");
	printf("Insert the value that you want to delete: ");
	scanf("%d", &key);
	while((c = getchar()) != EOF && c != '\n'){}
	deleteNode(key);
	enterToContinue();
}

int main(){
	//main menu
	int choice = -1;
	do{
		system("cls");
		printf("Choose:\n"
		"1. Insert new value\n"
		"2. Change order\n"
		"3. Delete a value\n"
		"4. View B-Tree\n"
		"5. Find a value\n"
		"6. Exit\n>> ");
		scanf("%d", &choice);
		while((c = getchar()) != EOF && c != '\n'){}
		switch(choice){
		case 1:
			insertNewValue();
			break;
		case 2:
			changeOrder();
			break;
		case 3:
			deleteAValue();
			break;
		case 4:
			printAll();
			break;
		case 5:
			findAValue();
			break;
		}
	}while(choice != 6);
	
	return 0;
}
